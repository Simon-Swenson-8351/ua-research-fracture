import wx
import wx.lib.mixins.listctrl as wxlcmixins
from Util import *

class ViewRoad(wx.Panel, wxlcmixins.ColumnSorterMixin):

    def __init__(self, parent, mainWindow):
        wx.Panel.__init__(self, parent)

        self.mainWindow = mainWindow

        self.sortCol = 0
        # 0 is ascending, 1 is descending
        self.sortDir = 0
        self.roadName = None
        self.selectedNodeId = None
        self.fireOnItemSelected = True
        self.itemDataMap = {}

        sizer = wx.BoxSizer(orient = wx.VERTICAL)

        self.selectedRoadLabel = wx.StaticText(parent = self, label = 'Selected Road: None')
        self.nodeListCtrl = wx.ListCtrl(self, style = wx.LC_REPORT | wx.LC_SORT_ASCENDING | wx.LC_SINGLE_SEL)
        self.nodeListCtrl.InsertColumn(0, 'Node ID')
        self.nodeListCtrl.InsertColumn(1, 'Latitude')
        self.nodeListCtrl.SetColumnWidth(1, 120)
        self.nodeListCtrl.InsertColumn(2, 'Longitude')
        self.nodeListCtrl.SetColumnWidth(2, 120)
        self.nodeListCtrl.InsertColumn(3, 'Change')
        self.nodeListCtrl.SetColumnWidth(3, 120)

        sizer.Add(self.selectedRoadLabel, 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(self.nodeListCtrl, 1, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(sizer)
        sizer.Fit(self)

        wxlcmixins.ColumnSorterMixin.__init__(self, 4)
        self.OnSortOrderChanged = self.reselectItemAfterSort

        self.Bind(wx.EVT_LIST_COL_CLICK, self.onColClick, self.nodeListCtrl)
        self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.onItemSelected)

    def onColClick(self, event):
        event.Skip()

    def onItemSelected(self, event):
        if not self.fireOnItemSelected:
            return
        self.mainWindow.setSelectedNode(self.roadName, event.GetData())

    def reselectItemAfterSort(self):
        for i in range(self.nodeListCtrl.GetItemCount()):
            if self.nodeListCtrl.GetItemData(i) == self.selectedNodeId:
                self.nodeListCtrl.Select(i)

    def GetListCtrl(self):
        return self.nodeListCtrl

    def setSelectedRoad(self, roadName = None, road = None):
        self.roadName = roadName
        if roadName == None or road == None:
            self.selectedRoadLabel.SetLabel('Selected Road: None')
            self.nodeListCtrl.DeleteAllItems()
        else:
            self.selectedRoadLabel.SetLabel('Selected Road: ' + roadName)
            self.itemDataMap = {}
            for nodeId in range(len(road)):
                # TODO don't hard-code these difference values
                change = float(numpy.linalg.norm(
                    numpy.asarray(road[nodeId]['discreteTimestampValues'][0]['environmentVector']) -
                    numpy.asarray(road[nodeId]['discreteTimestampValues'][-1]['environmentVector'])
                ))
				# Zero padding is needed because Windows sorts the column lexicographically.
				# TODO zfill of 3 should not be hard-coded
                nodeAsList = [str(nodeId).zfill(3), road[nodeId]['lat'], road[nodeId]['longi'], change]
                self.itemDataMap[nodeId] = nodeAsList
                self.nodeListCtrl.Append(nodeAsList)
                self.nodeListCtrl.SetItemData(self.nodeListCtrl.GetItemCount() - 1, nodeId)

    def setSelectedNode(self, roadName = None, road = None, nodeId = None):
        if roadName == None and nodeId != None:
            raise Exception('A node ID was provided, but no corresponding road name was provided.')

        if roadName != self.roadName:
            self.setSelectedRoad(roadName, road)
        self.selectedNodeId = nodeId
        # Need some way to notify that onItemSelected shouldn't fire, or it 
        # will cause infinite recursion
        self.fireOnItemSelected = False
        if nodeId == None:
            self.nodeListCtrl.Select(self.nodeListCtrl.GetFirstSelected(), on = 0)
        else:
            for i in range(self.nodeListCtrl.GetItemCount()):
                if self.nodeListCtrl.GetItemData(i) == nodeId:
                    self.nodeListCtrl.Select(i)
        self.fireOnItemSelected = True