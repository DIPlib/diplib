import diplib as dip

# Replicate what ../cpp/oversegmentation.cpp does

input = dip.ImageReadICS("../trui.ics")
superpixels = dip.Superpixels(input, 0.01, 1.0, "CW", {"no gaps"})
msr = dip.MeasurementTool.Measure(superpixels, input, ["Mean"])
graph = dip.RegionAdjacencyGraph(superpixels, msr["Mean"], "touching")
graph = graph.MinimumSpanningForest([1])
graph.RemoveLargestEdges(80 - 1)  # Find 80 regions
output = dip.Relabel(superpixels, graph)

superpixels = dip.ObjectToMeasurement(superpixels, msr["Mean"])
msr = dip.MeasurementTool.Measure(output, input, ["Mean"])
output = dip.ObjectToMeasurement(output, msr["Mean"])

win1 = input.ShowSlice("input")
win2 = superpixels.ShowSlice("superpixels", link=win1)
win3 = output.ShowSlice("output", link=win2)
dip.viewer.Spin()
