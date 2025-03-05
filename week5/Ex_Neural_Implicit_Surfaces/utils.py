"""
@author: Thor Vestergaard Christiansen (s173949@dtu.dk)

Functions used in the deep learning framework
"""

from pygel3d import hmesh
import torch
torch.multiprocessing.set_sharing_strategy('file_system')
from torch.utils.data import Dataset
import torch.nn as nn
import torch.nn.functional as Func
import torch.nn.init as init
from torch.nn.parameter import Parameter
from torch.utils.data import Dataset, DataLoader
import torch.optim as optim
from bisect import bisect_left
import plotly.offline as py
import plotly.graph_objects as go
import matplotlib.pyplot as plt
from IPython import display
import os
import numpy as np
import math

# ------------------------------------------------------------------------------------------------------
# Latent Vector Class
# Purpose of function: To initialize and store the latent vectors for each shape, when approximating their SDF with the network
# 
# Arguments to the function
# mu:               float - The mean value of the normal distribution that the latent vectors are drawn from
# sigma:            float - The variance value of the normal distribution that the latent vectors are drawn from
# lv_size:          integer - the number of elements in the latent vector
# nr_meshes:        integer - the number of meshes
# device:           string - either 'cpu' or 'cuda'
#
# ------------------------------------------------------------------------------------------------------
class Latent_vectors:
    def __init__(self, mu : float, sigma : float, lv_size : int, nr_meshes : int, device : torch.device) -> None:
        self.mu = mu
        self.sigma = sigma
        self.latent_vector_size = lv_size
        self.latent_vectors = torch.normal(mu, sigma, size=(nr_meshes, self.latent_vector_size), requires_grad=True, device=device)     

    def __len__(self):
        return len(self.latent_vectors)    

# ------------------------------------------------------------------------------------------------------
# The neural network as a class
# Purpose of function: The neural network, which is used to approximate the SDF of each shape together with the latent vectors
# 
# Arguments to the function
# input_size:       integer - the input to the network (the size of the latent vector + xyz for the 3D point)
# hidden_layers:    list - The number of hidden units for every layer
# device:           torch.device - It is actually a string, which is either 'cuda:0' or 'cpu'
#
# ------------------------------------------------------------------------------------------------------
class Network(nn.Module):

    def __init__(self, input_size : int, hidden_layers : list, device : torch.device) -> None:
        super().__init__()  
        
        # input layer
        self.W1 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[0], input_size))) # Kaiming initialization
        self.c1 = Parameter(torch.max(torch.sum(torch.abs(self.W1),axis=1)))
        self.b1 = Parameter(init.constant_(torch.Tensor(hidden_layers[0]), 0))
        
        # 2nd layer
        self.W2 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[1], hidden_layers[0])))
        self.c2 = Parameter(torch.max(torch.sum(torch.abs(self.W2),axis=1)))
        self.b2 = Parameter(init.constant_(torch.Tensor(hidden_layers[1]), 0))
        
        # 3rd layer
        self.W3 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[2], hidden_layers[1])))
        self.c3 = Parameter(torch.max(torch.sum(torch.abs(self.W3),axis=1)))
        self.b3 = Parameter(init.constant_(torch.Tensor(hidden_layers[2]), 0))
        
        # 4th layer
        self.W4 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[3]-input_size, hidden_layers[2])))
        self.c4 = Parameter(torch.max(torch.sum(torch.abs(self.W4),axis=1)))
        self.b4 = Parameter(init.constant_(torch.Tensor(hidden_layers[3]-input_size), 0))
        
        # 5th layer
        # Reintroduce the latent vector
        self.W5 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[4], hidden_layers[3])))
        self.c5 = Parameter(torch.max(torch.sum(torch.abs(self.W5),axis=1)))
        self.b5 = Parameter(init.constant_(torch.Tensor(hidden_layers[4]), 0))
        
        # 6th layer
        self.W6 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[5], hidden_layers[4])))
        self.c6 = Parameter(torch.max(torch.sum(torch.abs(self.W6),axis=1)))
        self.b6 = Parameter(init.constant_(torch.Tensor(hidden_layers[5]), 0))
        
        # 7th layer
        self.W7 = Parameter(init.kaiming_normal_(torch.Tensor(hidden_layers[6], hidden_layers[5])))
        self.c7 = Parameter(torch.max(torch.sum(torch.abs(self.W7),axis=1)))
        self.b7 = Parameter(init.constant_(torch.Tensor(hidden_layers[6]), 0))

        # Final layer
        self.W8 = Parameter(init.kaiming_normal_(torch.Tensor(1, hidden_layers[7])))
        self.c8 = Parameter(torch.max(torch.sum(torch.abs(self.W8),axis=1)))
        self.b8 = Parameter(init.constant_(torch.Tensor(1),0))
        
        # Activatiaon functions
        self.relu_activation = torch.nn.ReLU() # ReLU activation function
        self.hyp_tan_activation = torch.nn.Tanh()
        self.softplus = torch.nn.Softplus()

        self.ones = torch.ones(1).to(device)
        
        
    def weight_normalization(self, W : torch.tensor, softplus_c : torch.tensor) -> torch.tensor:
        absrowsum = torch.sum(torch.abs(W), axis=1)
        scale = torch.minimum(self.ones, softplus_c/absrowsum)
        W.data = W.data*scale[:,None]
    
    # Function that forwards the input the network, compute the weight normalization and outputs the result
    def train_forward(self, x_input : torch.tensor) -> torch.tensor:
        self.weight_normalization(self.W1, self.softplus(self.c1))
        x = Func.linear(x_input, self.W1, self.b1) # First layer
        x = self.relu_activation(x)        # The activitation function is set to the ReLU function.

        self.weight_normalization(self.W2, self.softplus(self.c2))
        x = Func.linear(x, self.W2, self.b2) # First layer
        x = self.relu_activation(x)              # ReLU activation function

        self.weight_normalization(self.W3, self.softplus(self.c3))
        x = Func.linear(x, self.W3, self.b3) # Third layer
        x = self.relu_activation(x)              # ReLU activation function

        self.weight_normalization(self.W4, self.softplus(self.c4))
        x = Func.linear(x, self.W4, self.b4) # Fourth layer
        x = self.relu_activation(x)              # ReLU activation function

        self.weight_normalization(self.W5, self.softplus(self.c5))
        x = Func.linear(torch.cat((x, x_input), 1), self.W5, self.b5) # 5th layer
        x = self.relu_activation(x)              # ReLU activation function

        self.weight_normalization(self.W6, self.softplus(self.c6))
        x = Func.linear(x, self.W6, self.b6) # 6th layer
        x = self.relu_activation(x)              # ReLU activation function

        self.weight_normalization(self.W7, self.softplus(self.c7))
        x = Func.linear(x, self.W7, self.b7) # 7th layer
        x = self.relu_activation(x)              #  ReLU activation

        self.weight_normalization(self.W8, self.softplus(self.c8))
        x = Func.linear(x, self.W8, self.b8) # Final layer
        x = self.hyp_tan_activation(x) # Tangent hyperbolic activiation function
        return x
    
    # Function that computes the Lipschitz loss
    def get_lipshitz_loss(self) -> torch.tensor:
        loss_lip = self.ones
        loss_lip = torch.mul(loss_lip, self.softplus(self.c1))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c2))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c3))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c4))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c5))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c6))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c7))
        loss_lip = torch.mul(loss_lip, self.softplus(self.c8))
        return loss_lip
    
    # Function that normalizes the parameters to ensure that the network is Lipschitz continuous
    def normalize_params(self) -> None:
        self.weight_normalization(self.W1, self.softplus(self.c1))
        self.weight_normalization(self.W2, self.softplus(self.c2))
        self.weight_normalization(self.W3, self.softplus(self.c3))
        self.weight_normalization(self.W4, self.softplus(self.c4))
        self.weight_normalization(self.W5, self.softplus(self.c5))
        self.weight_normalization(self.W6, self.softplus(self.c6))
        self.weight_normalization(self.W7, self.softplus(self.c7))
        self.weight_normalization(self.W8, self.softplus(self.c8))

    # Function that is used to     
    def eval_forward(self, x_input : torch.tensor) -> torch.tensor:
        x = Func.linear(x_input, self.W1, self.b1) # First layer
        x = self.relu_activation(x)        # The activitation function is set to the ReLU function.

        x = Func.linear(x, self.W2, self.b2) # Second layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W3, self.b3) # Third layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W4, self.b4) # Fourth layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(torch.cat((x, x_input), 1), self.W5, self.b5) # 5th layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W6, self.b6) # 6th layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W7, self.b7) # 7th layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W8, self.b8) # Final layer
        x = self.hyp_tan_activation(x) # Tangent hyperbolic activiation function
        return x

    def forward(self, x_input : torch.tensor) -> torch.tensor:
        x = Func.linear(x_input, self.W1, self.b1) # First layer
        x = self.relu_activation(x)        # The activitation function is set to the ReLU function.

        x = Func.linear(x, self.W2, self.b2) # Second layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W3, self.b3) # Third layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W4, self.b4) # Fourth layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(torch.cat((x, x_input), 1), self.W5, self.b5) # 5th layer with skip connection
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W6, self.b6) # 6th layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W7, self.b7) # 7th layer
        x = self.relu_activation(x)              # ReLU activation function

        x = Func.linear(x, self.W8, self.b8) # Final layer
        x = self.hyp_tan_activation(x) # Tangent hyperbolic activiation function
        return x
    
# ------------------------------------------
# Function which scales the mesh
# The following function is mapping the positions of the vertices of the mesh, which is extracted from the 
# hmesh.volumetric_isocontour function, to the 3D space between the box given by [Plow, Phigh]. 
# ------------------------------------------
class XForm:

    # Initialize with the minimum coordinates corner Plow (3x1 numpy array), maximum coordinates corner Phigh (3x1 numpy array), 
    # and the voxel grid dimensions in a tuple e.g. (100, 100, 100) if the voxel grid is 100x100x100
    def __init__(self, Plow : float, Phigh : float, dims : tuple) -> None:
        self.dims = np.array(dims)
        self.Plow = np.array(Plow)
        self.Phigh = np.array(Phigh)
        self.scaling = (self.Phigh - self.Plow) / self.dims
        self.inv_scaling = self.dims / (self.Phigh - self.Plow)

    # map transforms a grid point in the (dims[0], dims[1], dims[2]) grid to the point in 3D space between Plow and Phigh
    # that corresponds to the grid point. """
    def map(self, pi : np.ndarray) -> np.ndarray:
        return self.scaling * np.array(pi) + self.Plow
    
    def inv_map(self, p : np.ndarray) -> np.ndarray:
        return np.array((p-self.Plow) * self.inv_scaling,dtype=int)
    

# ------------------------------------------
# Display functions
# The following functions are in charge of displaying the graphical content
# so we can visualize the mesh and the point cloud
# ------------------------------------------
def gelmesh_to_plotyly(m : hmesh.Manifold, c) -> go.Mesh3d:
    xyz = np.array([ p for p in m.positions()])
    m_tri = hmesh.Manifold(m)
    hmesh.triangulate(m_tri)
    ijk = np.array([[ idx for idx in m_tri.circulate_face(f,'v')] for f in m_tri.faces()])
    mesh = go.Mesh3d(x=xyz[:,0],y=xyz[:,1],z=xyz[:,2],
            i=ijk[:,0],j=ijk[:,1],k=ijk[:,2],color=c,flatshading=True,opacity=0.50)
    return mesh

# m0 and m1 are both PyGEL meshes
# ---------------------------------------------------------------------------- #
# The purpose of this function is to display two meshes together (one in red and the other in blue)
# 
# m0 :       PyGEL3D mesh - the first mesh that you want to display
# m1 :       PyGEL3D mesh - the second mesh that you want to display
# ---------------------------------------------------------------------------- #
def display_meshes(m0 : hmesh.Manifold, m1=None) -> None:
    mesh0 = gelmesh_to_plotyly(m0, '#0000dd')
    mesh_data = [mesh0]
    if m1 is not None:
        mesh1 = gelmesh_to_plotyly(m1, '#dd0000')
        mesh_data = [mesh0, mesh1]

    fig = go.FigureWidget(data=mesh_data)
    fig.layout.scene.aspectmode="data"
    fig.show(renderer="colab")
    py.iplot(fig)


# ---------------------------------------------------------------------------- #
# The purpose of this function is to display a mesh together with 3D points
# 
# m :       PyGEL3D mesh - the mesh that you want to display
# P :       numpy array - P is a Nx3 matrix, where N is the number of points.
# ---------------------------------------------------------------------------- #
def display_mesh_and_points(m : hmesh.Manifold, P : np.ndarray):
    if P.shape[1] != 3:
        raise ValueError("P must be Nx3")
    # Reshape to an array of size (1,3) if it is a single point
    if P.shape[0] == 1:
        P = P.reshape((1,3))

    trace = go.Scatter3d(
        x = P[:,0],
        y = P[:,1],
        z = P[:,2],
        mode = 'markers'
    )
    mesh = gelmesh_to_plotyly(m, '#dd0000')
    data = [trace, mesh]
    fig = go.FigureWidget(data=data)
    fig.layout.scene.aspectmode="data"
    fig.show(renderer="colab")
    py.iplot(fig)

# ---------------------------------------------------------------------------- #
# The purpose of this function is to display 3D points
# 
# P :       numpy array - P is a Nx3 matrix, where N is the number of points.
# ---------------------------------------------------------------------------- #
def display_points(P : np.ndarray):
    trace = go.Scatter3d(
        x = P[:,0],
        y = P[:,1],
        z = P[:,2],
        mode = 'markers'
    )
    data = [trace]
    fig = go.FigureWidget(data=data)
    fig.layout.scene.aspectmode="data"
    fig.show(renderer="colab")
    py.iplot(fig)