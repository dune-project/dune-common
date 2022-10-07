# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import os
try:
    s = os.environ['DUNEPY_BLOCK_PLOTTING']
    block = s in ['TRUE','True','true', '1', 't', 'y', 'yes']
except KeyError:
    block = True
try:
    s = os.environ['DUNEPY_DISABLE_PLOTTING']
    disable = s in ['TRUE','True','true', '1', 't', 'y', 'yes']
except KeyError:
    disable = False
block = block and (not disable)

try:
    import matplotlib
    from matplotlib import pyplot
    from matplotlib.collections import PolyCollection
    import numpy as np
    from numpy import amin, amax, linspace, linalg, random
    _addPlot = True

    def _plotGrid(fig, grid, gridLines="black"):
        for p in grid.polygons():
            coll = PolyCollection(p, facecolor='none', edgecolor=gridLines, linewidth=0.5, zorder=2)
            pyplot.gca().add_collection(coll)

    def _plotData(fig, grid, solution, level=0, gridLines="black",
            component=None, vectors=None, nofVectors=None,
            xlim=None, ylim=None, clim=None, cmap=None, colorbar=True,
            on="cell"):

        if (gridLines is not None) and (gridLines != ""):
            _plotGrid(fig, grid, gridLines=gridLines)

        if solution is not None:
            if on == "points":
                assert not any(gt.isNone for gt in grid.indexSet.types(0)), "Can't plot point data with polygonal grids, use `on=\"cells\" in plotting command"
                triangulation = grid.triangulation(level)
                data = solution.pointData(level)
                try:
                    x1 = vectors[0]
                    x2 = vectors[1]
                    if x1 >= solution.dimRange or x2 >= solution.dimRange:
                        vectors = None
                except:
                    vectors = None

                if not vectors == None:
                    if nofVectors==0:
                        nofVector = len(triangulation.x)
                    idx = random.randint(len(triangulation.x),size=nofVectors)
                    pyplot.quiver(triangulation.x[idx], triangulation.y[idx],
                              data[idx][:,x1], data[idx][:,x2],
                              units='xy', scale=10., zorder=3, color='blue',
                              width=0.007, headwidth=3., headlength=4.)
                else:
                    if component is None:
                      if solution.dimRange > 1:
                         data = linalg.norm(data,axis=1)
                      else:
                          data = data[:,0]
                    else:
                      data = data[:,component]
                    minData = amin(data)
                    maxData = amax(data)
                    if clim == None:
                        clim = [minData, maxData]
                    if clim[0] > minData and clim[1] < maxData:
                        extend = 'both'
                    elif clim[0] > minData:
                        extend = 'min'
                    elif clim[1] < maxData:
                        extend = 'max'
                    else:
                        extend = 'neither'
                    norm = matplotlib.colors.Normalize(vmin=clim[0], vmax=clim[1])
                    levels = linspace(clim[0], clim[1], 256, endpoint=True)
                    pyplot.tricontourf(triangulation, data, cmap=cmap, levels=levels,
                                    extend=extend, norm=norm)

                if colorbar is not None and colorbar:
                    if isinstance(colorbar,bool):
                        colorbar="vertical"
                    # having extend not 'both' does not seem to work (needs fixing)...
                    v = linspace(clim[0], clim[1], 10, endpoint=True)
                    cbar = pyplot.colorbar(orientation=colorbar,shrink=1.0, ticks=v)
                    cbar.ax.tick_params(labelsize=18)
            else:
                if not vectors is None: raise ValueError("polygonal data can not plot vector")
                polys, values = solution.polygonData()
                data = []
                for vv in values:
                    for v in vv:
                        if component is None:
                            if solution.dimRange > 1:
                                d = linalg.norm(v)
                            else:
                                d = v[0]
                        else:
                            d = v[component]
                        data += [d]

                data = np.array(data)
                minData = amin(data)
                maxData = amax(data)
                if clim == None:
                    clim = [minData, maxData]
                vert = []
                for p in polys:
                    for q in p:
                        vert.append(q)
                coll = PolyCollection(vert, array=data, edgecolor=gridLines, linewidth=0.5, zorder=2, cmap=cmap)
                pyplot.gca().add_collection(coll)
                if colorbar is not None and colorbar:
                    if isinstance(colorbar,bool):
                        colorbar="vertical"
                    # having extend not 'both' does not seem to work (needs fixing)...
                    if clim[0] > minData and clim[1] < maxData:
                        extend = 'both'
                    elif clim[0] > minData:
                        extend = 'min'
                    elif clim[1] < maxData:
                        extend = 'max'
                    else:
                        extend = 'neither'
                    v = linspace(clim[0], clim[1], 10, endpoint=True)
                    norm = matplotlib.colors.Normalize(vmin=clim[0], vmax=clim[1])
                    cbar = pyplot.colorbar(coll, orientation=colorbar,shrink=1.0,
                                  # extend=extend, norm=norm,
                                  ticks=v)
                    cbar.ax.tick_params(labelsize=18)

        fig.gca().set_aspect('equal')
        fig.gca().autoscale()
        if xlim:
            fig.gca().set_xlim(xlim)
        if ylim:
            fig.gca().set_ylim(ylim)

except ImportError or ModuleNotFoundError:
    _addPlot = False


def plotGrid(grid, gridLines="black", figure=None,
        xlim=None, ylim=None, figsize=None):
    if not _addPlot:
        print("plotting disabled since 'matplotlib' could not be imported")
        return

    if disable: return

    if figure is None:
        figure = pyplot.figure(figsize=figsize)
        show = True
    else:
        try:
            subPlot = figure[1]
            figure = figure[0]
            pyplot.subplot(subPlot)
        except:
            pass
        show = False

    _plotGrid(figure, grid, gridLines=gridLines)

    figure.gca().set_aspect('equal')
    figure.gca().autoscale()
    if xlim:
        figure.gca().set_xlim(xlim)
    if ylim:
        figure.gca().set_ylim(ylim)

    if show:
        pyplot.show(block=block)



def plotPointData(solution, level=0, gridLines="black",
        vectors=None, nofVectors=None, figure=None,
        xlim=None, ylim=None, clim=None, figsize=None, cmap=None,
        colorbar=True):
    if not _addPlot:
        print("plotting disabled since 'matplotlib' could not be imported")
        return

    if disable: return
    try:
        grid = solution.grid
    except:
        grid = solution
        solution = None
    if not grid.dimension == 2:
        raise ValueError("inline plotting so far only available for 2d grids")

    if figure is None:
        figure = pyplot.figure(figsize=figsize)
        show = True
    else:
        try:
            subPlot = figure[1]
            figure = figure[0]
            pyplot.subplot(subPlot)
        except:
            pass
        show = False
    _plotData(figure,grid,solution,level,gridLines,None,
            vectors,nofVectors,xlim,ylim,clim,cmap,
            colorbar=colorbar,on="points")

    if show:
        pyplot.show(block=block)

def plotCellData(solution, level=0, gridLines="black",
        vectors=None, nofVectors=None, figure=None,
        xlim=None, ylim=None, clim=None, figsize=None, cmap=None,
        colorbar=True):
    if not _addPlot:
        print("plotting disabled since 'matplotlib' could not be imported")
        return

    if disable: return
    try:
        grid = solution.grid
    except:
        grid = solution
        solution = None
    if not grid.dimension == 2:
        raise ValueError("inline plotting so far only available for 2d grids")

    if figure is None:
        figure = pyplot.figure(figsize=figsize)
        show = True
    else:
        try:
            subPlot = figure[1]
            figure = figure[0]
            pyplot.subplot(subPlot)
        except:
            pass
        show = False
    _plotData(figure,grid,solution,level,gridLines,None,vectors,nofVectors,xlim,ylim,clim,cmap,
            colorbar=colorbar,on="cells")

    if show:
        pyplot.show(block=block)

def plotComponents(solution, level=0, show=None, gridLines="black", figure=None,
        xlim=None, ylim=None, clim=None, figsize=None, cmap=None):
    if not _addPlot:
        print("plotting disabled since 'matplotlib' could not be imported")
        return

    if disable: return
    try:
        grid = solution.grid
    except:
        grid = solution
        solution = None
    if not grid.dimension == 2:
        raise ValueError("inline plotting so far only available for 2d grids")

    if not show:
        show = range(solution.dimRange)

    if figure is None:
        figure = pyplot.figure(figsize=figsize)
    offset = 1 if (gridLines is not None) and (gridLines != "") else 0
    subfig = 101+(len(show)+offset)*10

    # first the grid if required
    if (gridLines is not None) and (gridLines != ""):
        pyplot.subplot(subfig)
        _plotData(figure,grid,None,level,gridLines,None,False,None,xlim,ylim,clim,cmap,
                on="points")

    # add the data
    for p in show:
        pyplot.subplot(subfig+offset+p)
        _plotData(figure,grid,solution,level,"",p,False,None,xlim,ylim,clim,cmap,False,
                on="points")

    pyplot.show(block=block)

def plot(solution,*args,**kwargs):
    if not _addPlot:
        print("plotting disabled since 'matplotlib' could not be imported")
        return

    if disable: return
    try:
        grid = solution.grid
    except:
        grid = solution
    defaultOn = "cells" if any(gt.isNone for gt in grid.indexSet.types(0)) else "points"
    use = kwargs.pop("on",defaultOn)
    if use == "points":
        plotPointData(solution,*args,**kwargs)
    elif use == "components-points":
        plotComponents(solution,*args,**kwargs)
    elif use == "cells":
        plotCellData(solution,*args,**kwargs)
    else:
        raise ValueError("wrong value for 'on' parameter should be one of 'points','cells','components-points'")

def mayaviPointData(solution, level=0, component=0):
    if disable: return
    grid = solution.grid
    from mayavi import mlab
    triangulation = grid.triangulation(level)
    z = solution.pointData(level)[:,component]
    s = mlab.triangular_mesh(triangulation.x, triangulation.y, z,
                                triangulation.triangles)
    mlab.show(block=block)
