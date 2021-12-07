import wx
import os
import json
import Config
import Util
from ViewLocalImages import *
from ViewRoad import *
from ViewMap import *
from ViewNode import *

# Hierarchy:
# TODO this is a work in progress
# TDFrameMain
#   splitter
#     TDPanelMap
#     TDPanelList
# TDFrameImageDiff

class TDFrameMain(wx.Frame):

    def __init__(self):
        wx.Frame.__init__(self, None, title = 'SSTerrainDiffVis', size = Config.initialWindowSize)

        self.loadData()

        self.selectedRoadName = None
        self.selectedNodeId = None

        self.outerSplitter = wx.SplitterWindow(self, style = wx.SP_3D | wx.SP_LIVE_UPDATE)
        self.outerSplitter.SetSashGravity(Config.mapFrameProportion)

        self.viewMap = ViewMap(self.outerSplitter, self)

        rightPanel = wx.Panel(self.outerSplitter)
        rightPanelSizer = wx.BoxSizer(orient = wx.VERTICAL)
        self.viewRoad = ViewRoad(rightPanel, self)
        self.viewNode = ViewNode(rightPanel, self)
        rightPanelSizer.Add(self.viewRoad, 1, wx.ALL | wx.EXPAND)
        rightPanelSizer.Add(self.viewNode, 0, wx.ALL | wx.EXPAND)
        rightPanel.SetSizer(rightPanelSizer)
        rightPanelSizer.Fit(rightPanel)

        self.outerSplitter.SplitVertically(self.viewMap, rightPanel)
        self.outerSplitter.SetSashPosition(int(Config.initialWindowSize.GetWidth() * Config.mapFrameProportion))

        self.viewLocalImages = None

        self.Centre()

    def loadData(self):
        # Satellite image is loaded in the map subpanel, since it's not used 
        # anywhere else.

        # Road paths
        self.roads = { }

        for roadFilename in os.listdir(os.path.join(Config.dataFolder, Config.roadFolder)):
            if not roadFilename.endswith('.json'):
                continue
            with open(os.path.join(Config.dataFolder, Config.roadFolder, roadFilename), 'r') as roadFile:
                roadObj = json.load(roadFile)
                self.roads[roadFilename[:-5]] = roadObj

        # Image data. We load it here so that it only loads once instead of 
        # being loaded every time a ViewLocalImages is created.
        self.imageMetadata = {}
        for dire in os.listdir(os.path.join(Config.dataFolder, Config.cameraImageFolder)):
            qualifiedPath = os.path.join(Config.dataFolder, Config.cameraImageFolder, dire)
            if os.path.isfile(qualifiedPath):
                continue
            self.imageMetadata[dire] = []
            for filename in os.listdir(qualifiedPath):
                if not filename.endswith('.json'):
                    continue
                with open(os.path.join(qualifiedPath, filename), 'r') as metadataFile:
                    metadata = json.load(metadataFile)
                    metadata['folder'] = dire
                    self.imageMetadata[dire].append(metadata)

    def setSelectedRoad(self, roadName = None):
        if roadName == None:
            self.selectedRoadName = None
            road = None
        else:
            self.selectedRoadName = roadName
            road = self.roads[roadName]
        self.viewMap.panelMap.setSelectedRoad(roadName)
        self.viewRoad.setSelectedRoad(roadName, road)

    def setSelectedNode(self, roadName = None, nodeId = None):
        if roadName == None and nodeId != None:
            raise Exception('A node ID was provided, but no corresponding road name was provided.')

        if roadName == None:
            self.selectedRoadName = None
            road = None
        else:
            self.selectedRoadName = roadName
            road = self.roads[roadName]
        if nodeId == None:
            self.selectedNodeId = None
            node = None
        else:
            self.selectedNodeId = nodeId
            node = self.roads[roadName][nodeId]
        self.viewMap.panelMap.setSelectedNode(roadName, nodeId)
        self.viewRoad.setSelectedNode(roadName, road, nodeId)
        self.viewNode.setSelectedNode(nodeId, node)

    # nodeId must be set to a valid value
    def showImageData(self):
        if self.selectedNodeId == None or self.selectedRoadName == None:
            raise Exception('Displaying images for an unset road or node')
        node = self.roads[self.selectedRoadName][self.selectedNodeId]
        if self.viewLocalImages != None:
            # Need a try clause in case the window was closed by the user 
            # previously, in which case we shouldn't close again.
            try:
                self.viewLocalImages.Close()
            except:
                pass
        self.viewLocalImages = ViewLocalImages(self, self.imageMetadata, node['lat'], node['longi'])
        self.viewLocalImages.Show()

if __name__ == '__main__':
    app = wx.App()
    mainFrame = TDFrameMain()
    mainFrame.Show()
    app.MainLoop()