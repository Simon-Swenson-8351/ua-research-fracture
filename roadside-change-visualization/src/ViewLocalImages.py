import wx
import wx.lib.scrolledpanel as wxsp
import os
import json
import numpy
import sys
import subprocess
import Config

class ViewLocalImages(wx.Frame):

    def __init__(self, parent, imageMetadata, lat, longi):
        wx.Frame.__init__(self, parent, title = 'Local Image View')

        self.imageMetadata = imageMetadata
        self.lat = lat
        self.longi = longi

        self.topImagesMetadata = []
        self.botImagesMetadata = []
        self.buttonList = []

        self.SetSize(Config.initialWindowSize)
        
        self.outerSizer = outerSizer = wx.BoxSizer(orient = wx.VERTICAL)

        self.locationLabel = wx.StaticText(self, label = 'No node selected. Select a node using the other view(s) first.', style = wx.ALIGN_CENTRE_HORIZONTAL)
        self.upperPanel = wxsp.ScrolledPanel(self)
        self.upperPanelSizer = wx.BoxSizer(orient = wx.HORIZONTAL)
        self.lowerPanel = wxsp.ScrolledPanel(self)
        self.lowerPanelSizer = wx.BoxSizer(orient = wx.HORIZONTAL)

        self.populateImageDisplays()

        self.outerSizer.Add(self.locationLabel, 0, wx.ALL | wx.EXPAND, 5)
        self.outerSizer.Add(self.upperPanel, 1, wx.ALL | wx.EXPAND, 5)
        self.outerSizer.Add(self.lowerPanel, 1, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(self.outerSizer)
        #self.outerSizer.Fit(self)

        self.Bind(wx.EVT_BUTTON, self.onClick)
        

    def onClick(self, event):
        imagesMetadata = self.topImagesMetadata + self.botImagesMetadata
        for i in range(len(self.buttonList)):
            if event.GetEventObject() == self.buttonList[i]:
                fullPath = os.path.join(
                    Config.dataFolder,
                    Config.cameraImageFolder,
                    imagesMetadata[i]['folder'],
                    imagesMetadata[i]['image_filename'])
                if sys.platform.startswith('darwin'):
                    subprocess.call(['open', fullPath])
                elif os.name == 'nt':
                    os.startfile(fullPath)
                elif os.name == 'posix':
                    subprocess.call(['xdg-open', fullPath])

    def populateImageDisplays(self):
        latLongVec = numpy.array([self.lat, self.longi])
        for key in self.imageMetadata:
            # TODO these distances definitely need some cutoff distance, or 
            # we could end up with images from a dataset of a completely 
            # different road being displayed.
            closestTopImage = None
            closestTopDistance = None
            closestBotImage = None
            closestBotDistance = None
            for imageMetadata in self.imageMetadata[key]:
                if imageMetadata['direction'] == 0:
                    if closestTopImage == None:
                        closestTopImage = imageMetadata
                        closestTopDistance = numpy.linalg.norm(
                            latLongVec -
                            numpy.array([closestTopImage['latitude'], closestTopImage['longitude']])
                        )
                    else:
                        candidateDistance = numpy.linalg.norm(
                            latLongVec -
                            numpy.array([imageMetadata['latitude'], imageMetadata['longitude']])
                        )
                        if candidateDistance < closestTopDistance:
                            closestTopImage = imageMetadata
                            closestTopDistance = candidateDistance
                else:
                    if closestBotImage == None:
                        closestBotImage = imageMetadata
                        closestBotDistance = numpy.linalg.norm(
                            latLongVec -
                            numpy.array([closestBotImage['latitude'], closestBotImage['longitude']])
                        )
                    else:
                        candidateDistance = numpy.linalg.norm(
                            latLongVec -
                            numpy.array([imageMetadata['latitude'], imageMetadata['longitude']])
                        )
                        if candidateDistance < closestBotDistance:
                            closestBotImage = imageMetadata
                            closestBotDistance = candidateDistance
            if closestTopImage != None:
                self.topImagesMetadata.append(closestTopImage)
            if closestBotImage != None:
                self.botImagesMetadata.append(closestBotImage)

        self.locationLabel.SetLabel('Images local to (lat: ' + str(self.lat) + ', long: ' + str(self.longi) + ')')
        
        for topImageMetadata in self.topImagesMetadata:
            self.putImagePanel(self.upperPanel, self.upperPanelSizer, topImageMetadata)
        self.upperPanelSizer.Fit(self.upperPanel)

        for botImageMetadata in self.botImagesMetadata:
            self.putImagePanel(self.lowerPanel, self.lowerPanelSizer, botImageMetadata)
        self.lowerPanelSizer.Fit(self.lowerPanel)

        self.outerSizer.Fit(self)

    def putImagePanel(self, outerPanel, outerSizer, imageMetadata):
        innerPanel = wx.Panel(outerPanel, style = wx.SIMPLE_BORDER)

        innerSizer = wx.BoxSizer(orient = wx.VERTICAL)

        innerLabel = wx.StaticText(innerPanel, label = imageMetadata['image_filename'], style = wx.ALIGN_CENTRE_HORIZONTAL)

        innerImg = wx.Image(os.path.join(
            Config.dataFolder,
            Config.cameraImageFolder,
            imageMetadata['folder'],
            imageMetadata['image_filename']))
        innerImg.Rescale(int(round(innerImg.GetWidth() / (innerImg.GetHeight() / 240))), 240)
        innerBmp = innerImg.ConvertToBitmap()
        innerImage = wx.StaticBitmap(
            innerPanel,
            bitmap = innerBmp,
            size = wx.Size(innerBmp.GetWidth(), innerBmp.GetHeight()))

        innerButton = wx.Button(innerPanel, label = 'Open Image')
        self.buttonList.append(innerButton)

        innerSizer.Add(innerLabel, 0, wx.ALL | wx.EXPAND, 5)
        innerSizer.Add(innerImage, 0, wx.ALL, 5)
        innerSizer.Add(innerButton, 0, wx.ALL | wx.EXPAND, 5)

        innerPanel.SetSizer(innerSizer)
        innerSizer.Fit(innerPanel)

        outerSizer.Add(innerPanel, 0, wx.ALL | wx.EXPAND, 5)
