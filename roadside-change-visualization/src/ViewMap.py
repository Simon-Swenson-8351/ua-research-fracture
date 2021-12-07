import wx
import json
import os
import math
import datetime
import numpy
import time
import Config
import Util

class ViewMap(wx.Panel):

    def __init__(self, parent, mainWindow):
        wx.Panel.__init__(self, parent)

        sizer = wx.BoxSizer(orient = wx.VERTICAL)

        self.panelLookup = PanelLookup(self)
        sizer.Add(self.panelLookup, 0, wx.ALL | wx.EXPAND, 5)

        #self.panelTimeframe = PanelTimeframe(self)
        #sizer.Add(self.panelTimeframe, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(wx.StaticLine(self, style = wx.LI_HORIZONTAL), 0, wx.EXPAND, 5)

        self.panelMap = PanelMap(self, mainWindow)

        self.panelLookup.textCtrlLat.SetValue(str(self.panelMap.getViewLat()))
        self.panelLookup.textCtrlLong.SetValue(str(self.panelMap.getViewLong()))

        sizer.Add(self.panelMap, 1, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(sizer)
        sizer.Fit(self)

        self.panelLookup.buttonLookup.Bind(wx.EVT_BUTTON, self.onCoordinateLookupClick)

    def onCoordinateLookupClick(self, event):
        lat = float(self.panelLookup.textCtrlLat.GetValue())
        longi = float(self.panelLookup.textCtrlLong.GetValue())
        self.panelMap.setViewCenter(lat, longi)


################################################################################
class PanelLookup(wx.Panel):

    def __init__(self, parent):
        wx.Panel.__init__(self, parent = parent, style = wx.SIMPLE_BORDER)

        outerSizer = wx.BoxSizer(orient = wx.VERTICAL)
        innerSizer = wx.BoxSizer(orient = wx.HORIZONTAL)

        outerSizer.Add(wx.StaticText(self, label = 'Coordinate Lookup', style = wx.ALIGN_CENTRE_HORIZONTAL), 1, wx.ALL | wx.EXPAND, 5)

        staticTextLat = wx.StaticText(self, label = 'Latitude:')
        self.textCtrlLat = wx.TextCtrl(self)
        staticTextLong = wx.StaticText(self, label = 'Longitude:')
        self.textCtrlLong = wx.TextCtrl(self)
        self.buttonLookup = wx.Button(self, label = 'Go')

        innerSizer.Add(staticTextLat, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(self.textCtrlLat, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(staticTextLong, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(self.textCtrlLong, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(self.buttonLookup, 0, wx.ALL | wx.CENTRE, 5)

        outerSizer.Add(innerSizer)

        self.SetSizer(outerSizer)
        outerSizer.Fit(self)

################################################################################
class PanelTimeframe(wx.Panel):
    
    def __init__(self, parent):
        wx.Panel.__init__(self, parent = parent, style = wx.SIMPLE_BORDER)

        outerSizer = wx.BoxSizer(orient = wx.VERTICAL)
        innerSizer = wx.BoxSizer(orient = wx.HORIZONTAL)

        outerSizer.Add(wx.StaticText(self, label = 'Change Timeframe', style = wx.ALIGN_CENTRE_HORIZONTAL), 1, wx.ALL | wx.EXPAND, 5)

        staticTextStartDate = wx.StaticText(self, label = 'Start Date:')
        self.listBoxStartDate = wx.ComboBox(self, value = 'TODO', choices = ['TODO'], style = wx.CB_READONLY)
        staticTextEndDate = wx.StaticText(self, label = 'End Date:')
        self.listBoxEndDate = wx.ComboBox(self, value = 'TODO', choices = ['TODO'], style = wx.CB_READONLY)
        self.buttonUpdate = wx.Button(self, label = 'Update')

        innerSizer.Add(staticTextStartDate, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(self.listBoxStartDate, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(staticTextEndDate, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(self.listBoxEndDate, 0, wx.ALL | wx.CENTRE, 5)
        innerSizer.Add(self.buttonUpdate, 0, wx.ALL | wx.CENTRE, 5)

        outerSizer.Add(innerSizer)

        self.SetSizer(outerSizer)
        outerSizer.Fit(self)

################################################################################
class PanelMap(wx.Panel):

    def __init__(self, parent, mainWindow):
        wx.Panel.__init__(self, parent)

        self.mainWindow = mainWindow
        self.roads = self.mainWindow.roads

        self.SetDoubleBuffered(True)

        self.roadMaskBitmap = None
        self.roadMaskDc = wx.MemoryDC()
        self.nodeMaskBitmap = None
        self.nodeMaskDc = wx.MemoryDC()

        self.selectedRoad = None
        self.selectedNode = None

        self.satelliteImage = wx.Bitmap(os.path.join(Config.dataFolder, Config.satelliteImageryFolder, Config.satelliteImageryFilename)).ConvertToImage()
        with open(os.path.join(Config.dataFolder, Config.satelliteImageryFolder, Config.satteliteMetadataFilename), 'r') as satelliteImageMetadataFile:
            self.satelliteImageMetadata = json.load(satelliteImageMetadataFile)
        latLong = self.satelliteImageMetadata['extents']['latitudeLongitude']
        self.viewLatLong = {
            'latBot': latLong['latBot'],
            'latTop': latLong['latTop'],
            'long': (latLong['longLeft'] + latLong['longRight']) / 2,
            'longLeft': None,
            'longRight': None
        }
        self.satelliteImageScalerX = Util.AffineScaler(latLong['longLeft'], latLong['longRight'], 0, self.satelliteImage.GetWidth() - 1)
        self.satelliteImageScalerY = Util.AffineScaler(latLong['latTop'], latLong['latBot'], 0, self.satelliteImage.GetHeight() - 1)

        self.maxRoadDiff = Util.getMaxRoadDiff(self.roads)
        # Non-red channels are scaled by this value.
        self.roadDiffColorScaler = Util.AffineScaler(0, self.maxRoadDiff, 255, 0)

        self.initRoadAndNodeColorIdMaps(self.roads)

        self.panelMinimap = PanelMinimap(self, self.satelliteImage)


        # Misc event-related variables
        self.mouseLeftDown = False
        # Bind events
        self.Bind(wx.EVT_PAINT, self.onPaint)
        self.Bind(wx.EVT_SIZE, self.onResize)
        self.Bind(wx.EVT_LEFT_DOWN, self.onLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.onLeftUp)
        self.Bind(wx.EVT_MOTION, self.onMouseMotion)
        self.Bind(wx.EVT_MOUSEWHEEL, self.onMouseWheel)
        
    def onPaint(self, event):
        dc = wx.BufferedPaintDC(self)
        self.paint(dc)

    def onResize(self, event):
        # For some reason, this event fires when the panel is still being 
        # initialized. This means the width and height are 0, which ends up 
        # causing a div by 0 error further along the way. This check prevents
        # that.
        if event.GetSize().GetWidth() == 0 or event.GetSize().GetHeight() == 0:
            return
        self.updateViewSpaceScalers()
        self.updateCurrentSatelliteView()

    def onLeftDown(self, event):
        self.mouseLeftDown = True
        # WX docs say this is needed to pass on to change focus handler.
        event.Skip()
        self.clickLogicalPosition = event.GetLogicalPosition(wx.ClientDC(self))
        self.clickStartTime = time.time()

    def onLeftUp(self, event):
        pos = event.GetLogicalPosition(wx.ClientDC(self))
        if not self.mouseLeftDown or \
            time.time() - self.clickStartTime > 1.0 or \
            pos.x != self.clickLogicalPosition.x or \
            pos.y != self.clickLogicalPosition.y:

            return

        roadColorId = Util.colorToInt(self.roadMaskDc.GetPixel(pos.x, pos.y))
        nodeColorId = Util.colorToInt(self.nodeMaskDc.GetPixel(pos.x, pos.y))
        # No road selected
        if self.selectedRoad == None:
            if roadColorId != 0:
                self.mainWindow.setSelectedRoad(self.colorIdRoadMap[roadColorId])
        # Road selected, but no node selected
        elif self.selectedNode == None:
            if nodeColorId != 0:
                self.mainWindow.setSelectedNode(self.selectedRoad, self.colorIdNodeMaps[self.selectedRoad][nodeColorId])
            elif roadColorId == 0:
                self.mainWindow.setSelectedRoad()
            elif roadColorId != self.roadColorIdMap[self.selectedRoad]:
                self.mainWindow.setSelectedRoad(self.colorIdRoadMap[roadColorId])
        # Road and node selected
        else:
            if nodeColorId == 0:
                self.mainWindow.setSelectedNode(self.selectedRoad)
            elif nodeColorId != self.nodeColorIdMaps[self.selectedRoad][self.selectedNode]:
                self.mainWindow.setSelectedNode(self.selectedRoad, self.colorIdNodeMaps[self.selectedRoad][nodeColorId])
        self.mouseLeftDown = False

    def onMouseMotion(self, event):
        curMousePos = wx.GetMousePosition()
        if event.Dragging():
            # map should move opposite direction of drag, so subtract
            curMouseLat = self.screenSpaceScalerY.transform(curMousePos.y)
            curMouseLong = self.screenSpaceScalerX.transform(curMousePos.x)
            prevMouseLat = self.screenSpaceScalerY.transform(self.prevMousePos.y)
            prevMouseLong = self.screenSpaceScalerX.transform(self.prevMousePos.x)
            dLat = prevMouseLat - curMouseLat
            dLong = prevMouseLong - curMouseLong
            self.pan(dLat, dLong)
            
        self.prevMousePos = curMousePos
        

    def onMouseWheel(self, event):
        scaleFactor = Config.scrollWheelScaleFactor
        if event.GetWheelRotation() > 0:
            scaleFactor = 1 / scaleFactor
        self.zoom(scaleFactor)

    def getViewLat(self):
        return (self.viewLatLong['latBot'] + self.viewLatLong['latBot']) / 2

    def getViewLong(self):
        return self.viewLatLong['long']

    # Since this class has access to the road list, don't need to pass the road 
    # object, just the road name.
    def setSelectedRoad(self, roadName = None):
        oldRoad = self.selectedRoad
        self.selectedRoad = roadName
        if oldRoad != self.selectedRoad:
            self.forcePaint()

    # Since this class has access to the road list, don't need to pass the node 
    # object, just the node id.
    def setSelectedNode(self, roadName = None, nodeId = None):
        if roadName == None and nodeId != None:
            raise Exception('A node ID was provided, but no corresponding road name was provided.')
        oldRoad = self.selectedRoad
        self.selectedRoad = roadName
        oldNode = self.selectedNode
        self.selectedNode = nodeId
        if oldRoad != self.selectedRoad or oldNode != self.selectedNode:
            self.forcePaint()

    # This method does several things. First, it precomputes road path images 
    # and road node images. This is to speed up the paint method by preventing 
    # a redraw/recomputation of each road path and road node. Instead, we 
    # crop, scale, and overlay each road image on top of each other, 
    # Basically, we need an easy way to map a mouse location to which road or 
    # road node it's over. We use image masks for this purpose.
    def initRoadAndNodeColorIdMaps(self, roads):
        self.roadColorIdMap = {}
        self.colorIdRoadMap = {}
        self.nodeColorIdMaps = {}
        self.colorIdNodeMaps = {}

        roadColorId = (1 << 24) - 1
        for roadName, road in self.roads.items():
            # TODO I made this a bidirectional map in case we need to go the 
            # reverse direction. If that's never used, take it out.
            self.roadColorIdMap[roadName] = roadColorId
            self.colorIdRoadMap[roadColorId] = roadName

            self.nodeColorIdMaps[roadName] = {}
            self.colorIdNodeMaps[roadName] = {}
            roadNodeColorId = (1 << 24) - 1
            for roadNodeIdx in range(len(road)):
                self.colorIdNodeMaps[roadName][roadNodeColorId] = roadNodeIdx
                self.nodeColorIdMaps[roadName][roadNodeIdx] = roadNodeColorId
                roadNodeColorId -= 1
            roadColorId -= 1

    def setViewCenter(self, newLatCenter, newLongCenter):
        self.viewLatLong['long'] = newLongCenter
        curViewLatDelta = abs(self.viewLatLong['latTop'] - self.viewLatLong['latBot'])
        self.viewLatLong['latTop'] = newLatCenter + curViewLatDelta / 2
        self.viewLatLong['latBot'] = newLatCenter - curViewLatDelta / 2
        self.updateViewSpaceScalers()
        self.updateCurrentSatelliteView()

    def pan(self, dLat, dLong):
            self.viewLatLong['latBot'] = self.viewLatLong['latBot'] + dLat
            self.viewLatLong['latTop'] = self.viewLatLong['latTop'] + dLat
            self.viewLatLong['long'] = self.viewLatLong['long'] + dLong
            self.updateViewSpaceScalers()
            self.updateCurrentSatelliteView()
        
    def zoom(self, zoomFactor):
        latMid = (self.viewLatLong['latTop'] + self.viewLatLong['latBot']) / 2
        latDist = abs(self.viewLatLong['latTop'] - self.viewLatLong['latBot'])
        latDist = latDist * zoomFactor
        self.viewLatLong['latTop'] = latMid + latDist / 2
        self.viewLatLong['latBot'] = latMid - latDist / 2
        self.updateViewSpaceScalers()
        self.updateCurrentSatelliteView()

    def updateViewSpaceScalers(self):
        longLatRatio = Util.getLongLatRatio((self.viewLatLong['latTop'] + self.viewLatLong['latBot']) / 2)
        viewAspectRatio = self.GetSize().GetWidth() / self.GetSize().GetHeight()
        longLatAspectRatio = viewAspectRatio / longLatRatio
        latExtents = abs(self.viewLatLong['latBot'] - self.viewLatLong['latTop'])
        self.viewLatLong['longLeft'] = self.viewLatLong['long'] - latExtents / 2 * longLatAspectRatio
        self.viewLatLong['longRight'] = self.viewLatLong['long'] + latExtents / 2 * longLatAspectRatio
        self.screenSpaceScalerX = Util.AffineScaler(0, self.GetSize().GetWidth(), self.viewLatLong['longLeft'], self.viewLatLong['longRight'])
        self.screenSpaceScalerY = Util.AffineScaler(0, self.GetSize().GetHeight(), self.viewLatLong['latTop'], self.viewLatLong['latBot'])

    def updateCurrentSatelliteView(self):

        # Register screen space and satellite image space with lat/long space
        xLeft = int(round(self.satelliteImageScalerX.transform(self.viewLatLong['longLeft'])))
        xRight = int(round(self.satelliteImageScalerX.transform(self.viewLatLong['longRight'])))
        yTop = int(round(self.satelliteImageScalerY.transform(self.viewLatLong['latTop'])))
        yBot = int(round(self.satelliteImageScalerY.transform(self.viewLatLong['latBot'])))

        self.currentSatelliteViewImage = self.satelliteImage.Copy()
        # Crop the canvas of the satellite image to match the view
        newSize = wx.Size(xRight - xLeft, yBot - yTop)
        newPos = wx.Point(-xLeft, -yTop)
        self.currentSatelliteViewImage.Resize(newSize, newPos, 0, 0, 0)
        # Now stretch/shrink the image to fit the view
        self.currentSatelliteViewImage.Rescale(self.GetSize().GetWidth(), self.GetSize().GetHeight())
        self.currentSatelliteViewBitmap = self.currentSatelliteViewImage.ConvertToBitmap()
        self.forcePaint()

    def forcePaint(self):
        self.paint(wx.BufferedDC(wx.ClientDC(self)))
        self.panelMinimap.forcePaint()

    def paint(self, dc):
        dc.SetBackground(wx.Brush(wx.Colour(0, 0, 0)))
        dc.Clear()
        dc.DrawBitmap(self.currentSatelliteViewBitmap, 0, 0)

        # Road mask bitmap and node mask bitmap are always the same size, so 
        # this check counts for both of them.
        if self.roadMaskBitmap == None or self.roadMaskBitmap.GetWidth() != dc.GetSize().GetWidth() or self.roadMaskBitmap.GetHeight() != dc.GetSize().GetHeight():
            self.roadMaskBitmap = wx.Bitmap(dc.GetSize())
            self.roadMaskDc.SelectObject(self.roadMaskBitmap)
            self.nodeMaskBitmap = wx.Bitmap(dc.GetSize())
            self.nodeMaskDc.SelectObject(self.roadMaskBitmap)

        self.roadMaskDc.SetBackground(wx.Brush(wx.Colour(0, 0, 0)))
        self.roadMaskDc.Clear()
        self.nodeMaskDc.SetBackground(wx.Brush(wx.Colour(0, 0, 0)))
        self.nodeMaskDc.Clear()

        # TODO calculate difference based on the actual entered dates
        for roadName, road in self.roads.items():
            # Draw the links for all roads
            # Links background
            if roadName == self.selectedRoad:
                dc.SetPen(wx.Pen(Config.selectedColor, Config.roadLinkWidth + 2 * Config.roadLinkSelectedPenWidth))
            else:
                dc.SetPen(wx.Pen(wx.Colour(0, 0, 0), Config.roadLinkWidth + Config.roadLinkSelectedPenWidth))
            prevRoadPoint = None
            for roadLocation in road:
                roadPoint = wx.Point(
                    int(round(self.screenSpaceScalerX.inverse(roadLocation['longi']))),
                    int(round(self.screenSpaceScalerY.inverse(roadLocation['lat'])))
                )
                if prevRoadPoint != None:
                    dc.DrawLine(roadPoint.x, roadPoint.y, prevRoadPoint.x, prevRoadPoint.y)
                prevRoadPoint = roadPoint

            # Links foreground
            prevRoadPoint = None
            prevRoadPointColor = None
            self.roadMaskDc.SetPen(wx.Pen(Util.intToColor(self.roadColorIdMap[roadName]), Config.roadLinkWidth))
            for roadLocation in road:
                roadPoint = wx.Point(
                    int(round(self.screenSpaceScalerX.inverse(roadLocation['longi']))),
                    int(round(self.screenSpaceScalerY.inverse(roadLocation['lat'])))
                )

                # TODO this should not be hard-coded as taking the very first and very last image vectors
                roadLocationDiff = numpy.linalg.norm(
                    numpy.asarray(roadLocation['discreteTimestampValues'][0]['environmentVector']) -
                    numpy.asarray(roadLocation['discreteTimestampValues'][-1]['environmentVector'])
                )

                roadPointColor = wx.Colour(
                    255,
                    self.roadDiffColorScaler.transform(roadLocationDiff),
                    self.roadDiffColorScaler.transform(roadLocationDiff)
                )
                if prevRoadPoint != None:
                    mean = wx.Point(
                        (roadPoint.x + prevRoadPoint.x) // 2,
                        (roadPoint.y + prevRoadPoint.y) // 2
                    )

                    # Draw the visible road
                    dc.SetPen(wx.Pen(roadPointColor, Config.roadLinkWidth))
                    dc.DrawLine(roadPoint.x, roadPoint.y, mean.x, mean.y)
                    dc.SetPen(wx.Pen(prevRoadPointColor, Config.roadLinkWidth))
                    dc.DrawLine(mean.x, mean.y, prevRoadPoint.x, prevRoadPoint.y)

                    # Draw the road mask
                    self.roadMaskDc.DrawLine(roadPoint.x, roadPoint.y, mean.x, mean.y)
                    self.roadMaskDc.DrawLine(mean.x, mean.y, prevRoadPoint.x, prevRoadPoint.y)

                prevRoadPoint = roadPoint
                prevRoadPointColor = roadPointColor
            dc.SetPen(wx.Pen(wx.Colour(0, 0, 0), width = Config.roadPointSelectedPenWidth // 2))
            if self.selectedRoad != None and self.selectedRoad == roadName:
                for roadLocationIdx in range(len(road)):
                    roadLocation = road[roadLocationIdx]

                    # TODO this is a lot of repeated code from the loop above. Can it be refactored somehow?
                    roadPoint = wx.Point(
                        int(round(self.screenSpaceScalerX.inverse(roadLocation['longi']))),
                        int(round(self.screenSpaceScalerY.inverse(roadLocation['lat'])))
                    )

                    # TODO this should not be hard-coded as taking the very first and very last image vectors
                    roadLocationDiff = numpy.linalg.norm(
                        numpy.asarray(roadLocation['discreteTimestampValues'][0]['environmentVector']) -
                        numpy.asarray(roadLocation['discreteTimestampValues'][-1]['environmentVector'])
                    )

                    roadPointColor = wx.Colour(
                        255,
                        self.roadDiffColorScaler.transform(roadLocationDiff),
                        self.roadDiffColorScaler.transform(roadLocationDiff)
                    )

                    if self.selectedNode == roadLocationIdx:
                        dc.SetPen(wx.Pen(Config.selectedColor, width = Config.roadPointSelectedPenWidth))
                    dc.SetBrush(wx.Brush(roadPointColor))
                    if Config.nodeShape == 'circle':
                        dc.DrawCircle(roadPoint.x, roadPoint.y, Config.roadPointWidth // 2)
                    elif Config.nodeShape == 'square':
                        dc.DrawRectangle(
                            roadPoint.x - Config.roadPointWidth // 2,
                            roadPoint.y - Config.roadPointWidth // 2,
                            Config.roadPointWidth,
                            Config.roadPointWidth)
                    if self.selectedNode == roadLocationIdx:
                        dc.SetPen(wx.Pen(wx.Colour(0, 0, 0), width = Config.roadPointSelectedPenWidth // 2))

                    self.nodeMaskDc.SetPen(wx.Pen(Util.intToColor(self.nodeColorIdMaps[roadName][roadLocationIdx])))
                    self.nodeMaskDc.SetBrush(wx.Brush(Util.intToColor(self.nodeColorIdMaps[roadName][roadLocationIdx])))
                    if Config.nodeShape == 'circle':
                        self.nodeMaskDc.DrawCircle(roadPoint.x, roadPoint.y, Config.roadPointWidth // 2)
                    elif Config.nodeShape == 'square':
                        self.nodeMaskDc.DrawRectangle(
                            roadPoint.x - Config.roadPointWidth // 2,
                            roadPoint.y - Config.roadPointWidth // 2,
                            Config.roadPointWidth,
                            Config.roadPointWidth)
                

################################################################################
class PanelMinimap(wx.Panel):

    def __init__(self, parent, satelliteImage):
        # Width and height must be + 2 to account for the image
        wx.Panel.__init__(
            self,
            parent = parent,
            size = wx.Size(Config.minimapSize.GetWidth() + 2, Config.minimapSize.GetHeight() + 2),
            pos = wx.Point(5, 5),
            style = wx.SIMPLE_BORDER
        )
        self.panelMap = parent

        self.satelliteMinimapImage = satelliteImage.Copy()
        minimapAspectRatioSize = Util.scaleIntSizePreserveAspectRatio(self.satelliteMinimapImage.GetSize(), Config.minimapSize)
        self.satelliteMinimapImage.Rescale(minimapAspectRatioSize.GetWidth(), minimapAspectRatioSize.GetHeight())
        left = (Config.minimapSize.GetWidth() - self.satelliteMinimapImage.GetWidth()) // 2
        right = left + Config.minimapSize.GetWidth() - 1
        top = (Config.minimapSize.GetHeight() - self.satelliteMinimapImage.GetHeight()) // 2
        bot = top + Config.minimapSize.GetHeight() - 1
        latLong = parent.satelliteImageMetadata['extents']['latitudeLongitude']
        self.satelliteImageScalerX = Util.AffineScaler(latLong['longLeft'], latLong['longRight'], left, right)
        self.satelliteImageScalerY = Util.AffineScaler(latLong['latTop'], latLong['latBot'], top, bot)
        self.satelliteMinimapBitmap = self.satelliteMinimapImage.ConvertToBitmap()
        self.SetSize(wx.Size(self.satelliteMinimapBitmap.GetWidth() + 2, self.satelliteMinimapBitmap.GetHeight() + 2))

        self.Bind(wx.EVT_PAINT, self.onPaint)

    def onPaint(self, event = None):
        self.paint(wx.PaintDC(self))

    def forcePaint(self):
        self.paint(wx.ClientDC(self))

    def paint(self, dc):
        dc.Clear()
        dc.DrawBitmap(self.satelliteMinimapBitmap, 1, 1)
        if self.panelMap.viewLatLong != None:
            dc.SetPen(wx.Pen(wx.Colour(255, 0, 0)))
            dc.SetBrush(wx.Brush(wx.Colour(0, 0, 0), style = wx.TRANSPARENT))
            left = self.satelliteImageScalerX.transform(self.panelMap.viewLatLong['longLeft'])
            right = self.satelliteImageScalerX.transform(self.panelMap.viewLatLong['longRight'])
            top = self.satelliteImageScalerY.transform(self.panelMap.viewLatLong['latTop'])
            bot = self.satelliteImageScalerY.transform(self.panelMap.viewLatLong['latBot'])
            dc.DrawRectangle(left, top, right - left, bot - top)
        
        