# Wave Function Collapse Exercise
#
# Fill in the propagate and observe functions
# Complete the tasks at the end of the file
# Hand in short lab journal with 
# - the code that you wrote, 
# - answers to questions, and
# - the images you have produced.
# You hand in as a single PDF generated
# however you see fit.

from os import listdir
from os.path import isfile, join
from PIL import Image
import matplotlib.pyplot as plt
from numpy import zeros, ndindex, ones, argmin
from random import randint, shuffle, uniform
from queue import Queue, LifoQueue
from time import time
import numpy as np
from matplotlib.widgets import Button  # added at top if not already imported

# The East North West South vector contains index pairs for
# adjacent cells.
ENWS = [[1,0],[0,1],[-1,0],[0,-1]]


def ingrid(g, i, j):
    '''Check if i,j is inside the grid g'''
    return 0 <= i < g.shape[0] and 0 <= j < g.shape[1]

class Tile:
    NO_TILES = 0
    def __init__(self, img, N) -> None:
        self.img = img
        self.connections = zeros((4,N), dtype=int)
        self.n = Tile.NO_TILES
        Tile.NO_TILES += 1

    def show(self):
        '''Show is only for debugging purposes'''
        self.img.show()
        print(self.img.width, self.img.height)
        print(self.connections)

    def compatible_adjacency(self, t2, t2_idx):
        ''' Figure out if two tiles are compatible by checking row
        of pixels along adjacent edges.'''
        W,H = self.img.width, self.img.height
        pixels1 = [self.img.getpixel((W-1,y)) for y in range(H)]
        pixels2 = [t2.img.getpixel((0,y)) for y in range(H)]
        if pixels1 == pixels2:
            self.connections[0, t2_idx] = 1
        pixels1 = [self.img.getpixel((x,H-1)) for x in range(W)]
        pixels2 = [t2.img.getpixel((x,0)) for x in range(W)]
        if pixels1 == pixels2:
            self.connections[1, t2_idx] = 1
        pixels1 = [self.img.getpixel((0,y)) for y in range(H)]
        pixels2 = [t2.img.getpixel((W-1,y)) for y in range(H)]
        if pixels1 == pixels2:
            self.connections[2, t2_idx] = 1
        pixels1 = [self.img.getpixel((x,0)) for x in range(W)]
        pixels2 = [t2.img.getpixel((x,H-1)) for x in range(W)]
        if pixels1 == pixels2:
            self.connections[3, t2_idx] = 1

def load_tiles(exemplar):
    '''Load tiles from the specified tileset. Expand by creating
    rotated versions of each tileset'''
    path = 'tilesets/' + exemplar + '/'
    tiles = []
    fnames = [ f for f in listdir(path) if isfile(join(path, f)) ] 
    N = 4*len(fnames)
    for f in fnames:
        print(f)
        img = Image.open(join(path, f))
        tiles.append(Tile(img, N))
        tiles.append(Tile(img.transpose(Image.ROTATE_90), N))
        tiles.append(Tile(img.transpose(Image.ROTATE_180), N))
        tiles.append(Tile(img.transpose(Image.ROTATE_270), N))
    for t0 in tiles:
        for i, t1 in enumerate(tiles):
            t0.compatible_adjacency(t1,i)
    return tiles

# -------------------------------------
# Here the tile set is loaded, so change line below to try other
# tile set.
tiles = load_tiles("Castle")
# -------------------------------------

def pick_tile(we):
    '''from an array of possible tiles (0's and 1's) choose an entry containing 1
    and produce a new vector where only this entry is 1 and the rest are zeros'''
    l = []
    for i, wec in enumerate(we):
        if wec==1:
            l.append(i)
    x = randint(0,len(l)-1)
    we_new = zeros(len(we), dtype=int)
    we_new[l[x]] = 1
    return we_new

def wave_grid_to_image(wave_grid):
    '''Produce a displayable image from a wavegrid - possibly with superpositions.'''
    W = tiles[0].img.width
    H = tiles[0].img.height
    I = Image.new('RGBA',size=(W*wave_grid.shape[0], H*wave_grid.shape[1]), color=(255,255,255,255))
    N = len(tiles)
    for i,j in ndindex(wave_grid.shape[0:2]):
        entropy =  min(255.0,max(1.0,sum(wave_grid[i,j,:])+1e-6))
        mask = Image.new('RGBA', size=(W,H), color=(255, 255, 255, int(255/entropy)))
        for t_idx in range(N):
          if wave_grid[i, j, t_idx] == 1:
            I.paste(tiles[t_idx].img,  (W*i, H*j), mask)
    return I


def observe(wave_grid):
    '''The observe function picks a tile of minimum entropy and collapses
    its part of the wave function. A tuple with the tile indices is returned.'''
    # i, j = randint(0,wave_grid.shape[0]-1), randint(0,wave_grid.shape[1]-1)
    # Student code begin ---------------
    min_entropy = 1e6
    min_pos = None
    for i, j in ndindex(wave_grid.shape[0:2]):
        entropy = sum(wave_grid[i, j])
        if 1 < entropy < min_entropy:
            min_entropy = entropy
            min_pos = (i, j)
    if min_pos is None:
        raise Exception("No valid position found")
    i, j = min_pos
    
    # Student code end ---------------
    wave_grid[i, j, :] = pick_tile(wave_grid[i, j, :])
    return i, j

def get_posible_tiles(wave_grid, i, j, d):
    '''Get the possible tiles for a given position and direction using vectorized operations'''
    possible_tiles = np.zeros(len(tiles), dtype=bool)
    # Get indices where the tile is possible
    tile_indices = np.where(wave_grid[i, j, :] == 1)[0]
    for idx in tile_indices:
        # Use bitwise OR to combine the allowed connections
        possible_tiles |= (tiles[idx].connections[d] == 1)
    return possible_tiles.astype(int)

def propagate(wave_grid, i, j):
    '''Propagates the changes when a cell has been observed using a Python list as stack'''
    stack = [(i, j)]
    while stack:
        i, j = stack.pop()
        for d, offset in enumerate(ENWS):
            ni, nj = i + offset[0], j + offset[1]
            if not ingrid(wave_grid, ni, nj):
                continue

            possible_tiles = get_posible_tiles(wave_grid, i, j, d)
            current_entropy = np.sum(wave_grid[ni, nj])
            # Bitwise multiplication keeps only tiles that are allowed
            wave_grid[ni, nj] = wave_grid[ni, nj] * possible_tiles
            new_entropy = np.sum(wave_grid[ni, nj])
            if new_entropy < current_entropy:
                stack.append((ni, nj))
            
    # Student code end ---------------


def WFC(wave_grid, propagation_fn=propagate):
    try:
        i, j = observe(wave_grid)
        propagation_fn(wave_grid, i, j)
        return True
    except Exception:
        return False

def run_interactive(wave_grid, propagation_fn=propagate):
    '''This function runs WFC interactively, showing the result of each step. '''
    I = wave_grid_to_image(wave_grid)
    I.save("img0.png")
    fig = plt.figure()
    plt.ion()
    plt.imshow(I)
    plt.show(block=False)
    W = tiles[0].img.width
    H = tiles[0].img.height
    while WFC(wave_grid, propagation_fn):
        fig.clear()
        I = wave_grid_to_image(wave_grid)
        plt.imshow(I)
        plt.show(block=False)
        plt.pause(0.00000001)
    I.save("img1.png")


def run(wave_grid, propagation_fn=propagate):
    '''Run WFC non-interactively. Much faster since converting to 
    an image is the slowest part by far.'''
    I = wave_grid_to_image(wave_grid)
    I.save("img0.png")
    while WFC(wave_grid, propagation_fn):
        pass
    I = wave_grid_to_image(wave_grid)
    I.save("img1.png")

# Add a global persistent tile selection variable and a persistent tile window.
persistent_tile_selection = set()

def persistent_tile_window():
    """
    Opens a persistent side window that displays all tiles.
    Tiles can be toggled on/off (red border when selected).
    The current selection can be read from the global persistent_tile_selection.
    """
    global persistent_tile_selection
    fig = plt.figure("Tile Selection")
    persistent_tile_fig = fig  # store the figure handle for later shutdown
    num_tiles = len(tiles)
    ncols = 4
    nrows = int(np.ceil(num_tiles / ncols))
    axs = []
    for idx in range(num_tiles):
        ax = fig.add_subplot(nrows, ncols, idx+1)
        ax.imshow(tiles[idx].img)
        ax.set_xticks([])
        ax.set_yticks([])
        ax.text(0.5, 0.5, str(idx), transform=ax.transAxes,
                fontsize=12, color="blue", ha="center", va="center")
        # Initialize border
        ax.patch.set_linewidth(2)
        ax.patch.set_edgecolor("black")
        axs.append(ax)

    def on_tile_click(event):
        nonlocal axs
        for idx, a in enumerate(axs):
            if event.inaxes == a:
                if idx in persistent_tile_selection:
                    persistent_tile_selection.remove(idx)
                    a.patch.set_edgecolor("black")
                else:
                    persistent_tile_selection.add(idx)
                    a.patch.set_edgecolor("red")
                fig.canvas.draw_idle()
                break

    fig.canvas.mpl_connect("button_press_event", on_tile_click)
    plt.show(block=False)
    return persistent_tile_fig

# Modify precollapse_interactive to use the persistent tile window selection.
def precollapse_interactive(wave_grid):
    """
    Enables interactive precollapse: click a cell to precollapse.
    Use the persistent tile-selection window (always visible) to set the active tile.
    Press Enter (in the main window) to finish precollapse mode.
    """
    fig, ax = plt.subplots()
    I = wave_grid_to_image(wave_grid)
    im_obj = ax.imshow(I)
    ax.set_title("Precollapse Mode: click a cell to precollapse; press 'enter' to continue.")
    tile_w = tiles[0].img.width
    tile_h = tiles[0].img.height

    def on_click(event):
        if event.inaxes != ax:
            return
        col = int(event.xdata // tile_w)
        row = int(event.ydata // tile_h)
        print(f"Selected cell: ({row}, {col})")
        if len(persistent_tile_selection) == 0:
            print("No tile is activated in the persistent selection window. Skipping cell update.")
            return
        chosen_tile = list(persistent_tile_selection)
        print(f"Applied persistent tile: {chosen_tile}")
        new_vec = np.zeros(len(tiles), dtype=int)
        new_vec[chosen_tile] = 1
        wave_grid[col, row, :] = new_vec
        propagate(wave_grid, col, row)
        new_I = wave_grid_to_image(wave_grid)
        im_obj.set_data(new_I)
        fig.canvas.draw()

    def on_key(event):
        if event.key == "enter":
            print("Precollapse mode ended. Starting WFC.")
            fig.canvas.mpl_disconnect(cid_click)
            fig.canvas.mpl_disconnect(cid_key)
            plt.close(fig)

    cid_click = fig.canvas.mpl_connect("button_press_event", on_click)
    cid_key = fig.canvas.mpl_connect("key_press_event", on_key)
    plt.show(block=True)

# At startup, open the persistent tile-selection window.
# persistent_tile_fig = persistent_tile_window()

# # Part 1: When observe and propagate have been fixed, 
# # you can run the code below to produce a texture.
# # Try with a number of tilesets. The resulting images
# # are submitted. Try at least "Knots" and "FloorPlan"
# # Initialize the wave grid
# wave_grid = ones((25,25,len(tiles)), dtype=int)
# # Activate precollapse mode
# precollapse_interactive(wave_grid)
# if persistent_tile_fig:
#     plt.close(persistent_tile_fig)
# # Run interactive WFC using a selectable propagation function.
# # Change the second argument between 'propagate' and 'propagate_adjacent' as desired.
# run_interactive(wave_grid, propagation_fn=propagate)  # or use propagate_adjacent
# # run(wave_grid)

# Part 2: Introduce constraints by precollapsing one or more 
# cells to admit only one or more tiles in these cells. Discuss 
# what you are trying to achieve and submit the discussion along 
# with the resulting images.

## Precollapsing give as a power to obtain a random structure that still follows the person imagination. very nice for game level design.




# Part 3: Change your propagate function such that only adjacent
# cells are updated. Use this to produce a new texture based
# on FloorPlan. Does this change the result? If so how? Show
# images. 
def propagate_adjacent(wave_grid, i, j):
    '''Propagates the changes when a cell has been observed using a Python list as stack'''
    for d, offset in enumerate(ENWS):
        ni, nj = i + offset[0], j + offset[1]
        if not ingrid(wave_grid, ni, nj):
            continue

        possible_tiles = get_posible_tiles(wave_grid, i, j, d)
        # Bitwise multiplication keeps only tiles that are allowed
        wave_grid[ni, nj] = wave_grid[ni, nj] * possible_tiles

# Initialize the wave grid
wave_grid = ones((25,25,len(tiles)), dtype=int)
  
run_interactive(wave_grid, propagation_fn=propagate_adjacent)  # or use propagate_adjacent
 
# Part 4 (NON-MANDATORY AND PROBABLY HARD)
# Input a single image and make the tileset from patches in this
# image. See if you can produce results similar to Marie's
