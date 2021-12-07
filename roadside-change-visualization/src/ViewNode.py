import wx
from Util import *

class ViewNode(wx.Panel):

    def __init__(self, parent, mainWindow):
        wx.Panel.__init__(self, parent)

        self.mainWindow = mainWindow
        self.nodeId = None
        self.node = None

        sizer = wx.BoxSizer(orient = wx.VERTICAL)

        self.selectedNodeLabel = wx.StaticText(parent = self, label = 'Selected Node: None')
        self.latLabel = wx.StaticText(parent = self, label = '')
        self.longLabel = wx.StaticText(parent = self, label = '')
        self.changeFactorLabel = wx.StaticText(parent = self, label = '')
        self.viewLocalImageDataButton = wx.Button(self, label = 'View Local Image Data')
        self.viewLocalImageDataButton.Disable()
        self.jumpToNodeButton = wx.Button(self, label = 'Jump to Node on Map')
        self.jumpToNodeButton.Disable()

        sizer.Add(self.selectedNodeLabel, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.latLabel, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.longLabel, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.changeFactorLabel, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.viewLocalImageDataButton, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.jumpToNodeButton, 0, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(sizer)
        sizer.Fit(self)

        self.Bind(wx.EVT_BUTTON, self.onClick)

    def onClick(self, event):
        if event.GetEventObject() == self.viewLocalImageDataButton:
            self.mainWindow.showImageData()
        elif event.GetEventObject() == self.jumpToNodeButton:
            self.mainWindow.viewMap.panelMap.setViewCenter(self.node['lat'], self.node['longi'])

    def setSelectedNode(self, nodeId = None, node = None):
        if nodeId == None or node == None:
            self.selectedNodeLabel.SetLabel('Selected Node: None')
            self.latLabel.SetLabel('')
            self.longLabel.SetLabel('')
            self.changeFactorLabel.SetLabel('')
            self.viewLocalImageDataButton.Disable()
            self.jumpToNodeButton.Disable()
        else:
            self.nodeId = nodeId
            self.node = node
            self.selectedNodeLabel.SetLabel('Selected Node: ' + str(nodeId))
            self.latLabel.SetLabel('Latitude: ' + str(node['lat']))
            self.longLabel.SetLabel('Longitude: ' + str(node['longi']))
            change = float(numpy.linalg.norm(
                    numpy.asarray(node['discreteTimestampValues'][0]['environmentVector']) -
                    numpy.asarray(node['discreteTimestampValues'][-1]['environmentVector'])
                ))
            self.changeFactorLabel.SetLabel('Change Factor: ' + str(change))
            self.viewLocalImageDataButton.Enable()
            self.jumpToNodeButton.Enable()