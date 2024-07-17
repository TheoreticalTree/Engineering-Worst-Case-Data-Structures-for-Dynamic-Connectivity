import pandas
import matplotlib.pyplot as mp
import seaborn as sns

palette = {'DTree': 'yellow', 'OneForestConnect' : 'orange', 'HDT' : 'red',
           'GKKT[base]':'green', 'GKKT[pHeu]':'green', 'GKKT[lvlHeu]':'green',
           'Wang[base]':'blue', 'Wang[lvlHeu]':'blue', 'Wang[pHeu]':'blue',
           'Lazy[base]':'purple', 'Lazy[lylHeu]':'purple', 'Lazy[pHeu]':'purple'}
markers = {"GKKT[Base]": "o","GKKT[pHeu]": ".","GKKT[lvlHeu]": "*",
                  "Wang[Base]": "o","Wang[pHeu]": ".","Wang[lvlHeu]": "*",
                  "Lazy[Base]": "o","Lazy[pHeu]": ".","Lazy[lvlHeu]": "*",
                  "DTree": "o", "HDT": "o", "OneForestConnect": "o"}

df = pandas.read_csv('results/experiments_cutSet.csv')

plot = sns.relplot(df,
                   kind = 'line',
                   x = 'size',
                   y = 'probability')

plot.set(xscale="log")
plot.set(ylim=(0,1))
plot.set_axis_labels('number of edges in the cutSet', 'fraction of search succeeding')
#plot.set(title= "fraction of search succeeding / size of cutSet")

mp.savefig("plots/cutSetSuccesses.pdf")

mp.show()

# Here starts the regular plotting

df = pandas.read_csv('results/experiments_regular.csv')

testFrame = df[df["instance"].str.contains("cliques_small")]
testFrame.loc[:, "runtime"] = testFrame.loc[:, "runtime"].multiply(0.001)
testFrame.replace("Kaibel[Base]", "Lazy[Base]", inplace=True)
testFrame.replace("Kaibel[pHeu]", "Lazy[pHeu]", inplace=True)
testFrame.replace("Kaibel[lvlHeu]", "Lazy[lvlHeu]", inplace=True)

plot = sns.relplot(testFrame,
                        kind = 'scatter',
                        x = 'n',
                        y = 'runtime',
                        hue = 'algorithm',
                        palette = palette,
                        style='algorithm',
                        markers=markers,
                        legend=True)

plot.set(yscale="log")
plot.set(xscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/regular_cliques_small.pdf")

mp.show()

testFrame = df[df["instance"].str.contains("cliques_large")]
testFrame.loc[:, "runtime"] = testFrame.loc[:, "runtime"].multiply(0.001)
testFrame.replace("Kaibel[Base]", "Lazy[Base]", inplace=True)
testFrame.replace("Kaibel[pHeu]", "Lazy[pHeu]", inplace=True)
testFrame.replace("Kaibel[lvlHeu]", "Lazy[lvlHeu]", inplace=True)

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'n',
                   y = 'runtime',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set(xscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/regular_cliques_large.pdf")

mp.show()

testFrame = df[~df["instance"].str.contains("cliques_small") & ~df["instance"].str.contains("cliques_large")]
testFrame.loc[:, "runtime"] = testFrame.loc[:, "runtime"].multiply(0.001)
testFrame.replace("Kaibel[Base]", "Lazy[Base]", inplace=True)
testFrame.replace("Kaibel[pHeu]", "Lazy[pHeu]", inplace=True)
testFrame.replace("Kaibel[lvlHeu]", "Lazy[lvlHeu]", inplace=True)

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'instance',
                   y = 'runtime',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set_axis_labels('instance', 'time in ms')

mp.savefig("plots/regular_chen_instances.pdf")

mp.show()

# Here starts the maxUpdateTime plotting

df = pandas.read_csv('results/experiments_maxTimeUpdate.csv')

testFrame = df[df["instance"].str.contains("cliques_small")]
testFrame.loc[:, "maxTimeInsert"] = testFrame.loc[:, "maxTimeInsert"].multiply(0.001)
testFrame.loc[:, "maxTimeDelete"] = testFrame.loc[:, "maxTimeDelete"].multiply(0.001)
testFrame.replace("Kaibel[Base]", "Lazy[Base]", inplace=True)
testFrame.replace("Kaibel[pHeu]", "Lazy[pHeu]", inplace=True)
testFrame.replace("Kaibel[lvlHeu]", "Lazy[lvlHeu]", inplace=True)

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'n',
                   y = 'maxTimeInsert',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set(xscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/maxTimeInsert_cliques_small.pdf")

mp.show()

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'n',
                   y = 'maxTimeDelete',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set(xscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/maxTimeDelete_cliques_small.pdf")

mp.show()

testFrame = df[df["instance"].str.contains("cliques_large")]
testFrame.loc[:, "maxTimeInsert"] = testFrame.loc[:, "maxTimeInsert"].multiply(0.001)
testFrame.loc[:, "maxTimeDelete"] = testFrame.loc[:, "maxTimeDelete"].multiply(0.001)
testFrame.replace("Kaibel[Base]", "Lazy[Base]", inplace=True)
testFrame.replace("Kaibel[pHeu]", "Lazy[pHeu]", inplace=True)
testFrame.replace("Kaibel[lvlHeu]", "Lazy[lvlHeu]", inplace=True)

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'n',
                   y = 'maxTimeInsert',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set(xscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/maxTimeInsert_cliques_large.pdf")

mp.show()

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'n',
                   y = 'maxTimeDelete',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set(xscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/maxTimeDelete_cliques_large.pdf")

mp.show()

testFrame = df[~df["instance"].str.contains("cliques_small") & ~df["instance"].str.contains("cliques_large")]
testFrame.loc[:, "maxTimeInsert"] = testFrame.loc[:, "maxTimeInsert"].multiply(0.001)
testFrame.loc[:, "maxTimeDelete"] = testFrame.loc[:, "maxTimeDelete"].multiply(0.001)
testFrame.replace("Kaibel[Base]", "Lazy[Base]", inplace=True)
testFrame.replace("Kaibel[pHeu]", "Lazy[pHeu]", inplace=True)
testFrame.replace("Kaibel[lvlHeu]", "Lazy[lvlHeu]", inplace=True)

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'instance',
                   y = 'maxTimeInsert',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/maxTimeInsert_chen_instances.pdf")

mp.show()

plot = sns.relplot(testFrame,
                   kind = 'scatter',
                   x = 'instance',
                   y = 'maxTimeDelete',
                   hue = 'algorithm',
                   palette = palette,
                   style='algorithm',
                   markers=markers,
                   legend=True)

plot.set(yscale="log")
plot.set_axis_labels('number of vertices', 'time in ms')

mp.savefig("plots/maxTimeDelete_chen_instances.pdf")

mp.show()

# TODO add plots for the other types of  experiments

# generatedTests = df[df["instance"].str.contains("test.txt")]
#
# #Plotting by type of test
# for testType in {# "queryTest",# "partitionTest"
#                   }:
#     testFrame = generatedTests[generatedTests["type"].str.contains(testType)]
#     print(testFrame)
#     #Plotting within a specific size+density category
#     for density in {"4s", "20s", "4logn", "20logn", "4sqrtn", "20sqrtn"}:
#         plot = sns.relplot(testFrame[testFrame["density"] == density],
#                        kind = 'scatter',
#                        x = 'n',
#                        y = 'runtime',
#                        hue = 'algorithm',
#                        palette = palette,
#                        size='queryFrequency',
#                        legend=True)
#
#         plot.set(yscale="log")
#         plot.set(xscale="log")
#         plot.set_axis_labels('number of vertices', 'time in ms')
#         plot.set(title= testType + "/" + density)
#
#         mp.savefig("plots/dynConnectivity" + testType + density + ".svg")
#
#         mp.show()
#
#     plot = sns.relplot(data=testFrame,
#                     x = 'n',
#                     y = 'runtime',
#                     hue = 'algorithm',
#                     palette = palette,
#                     size='queryFrequency',
#                     style="density",
#                     markers=markersDensity,
#                     legend=True
#     )
#
#     plot.set(yscale="log")
#     plot.set(xscale="log")
#     plot.set_axis_labels('number of vertices', 'time in ms')
#     plot.set(title= testType)
#
#     mp.savefig("plots/dynConnectivity" + testType + "densityComparison" + ".svg")
#
#     mp.show()
#
# realTests = df[~df["instance"].str.contains("test.txt")]
#
# plot = sns.relplot(realTests[realTests["instance"].str.contains("bq")],
#                    kind = 'scatter',
#                    x = 'n',
#                    y = 'runtime',
#                    hue = 'algorithm',
#                    palette = palette,
#                    size='queryFrequency',
#                    legend=True)
#
# plot.set(xscale="log")
# plot.set(yscale="log")
# plot.set_xticklabels(rotation=-30)
# plot.set_axis_labels('n', 'time in ms')
# plot.set(title= "Blockqueries_Chen_Instances")
#
# mp.savefig("plots/Blockqueries_Chen_Instances.svg")
#
# mp.show()
#
# realTests = realTests[~realTests["instance"].str.contains("bq")]
#
# plot = sns.relplot(realTests[realTests["mode"] == "normal"],
#                    kind = 'scatter',
#                    x = 'n',
#                    y = 'runtime',
#                    hue = 'algorithm',
#                    palette = palette,
#                    size='queryFrequency',
#                    style='mode',
#                    markers=markersMode,
#                    legend=True)
#
# plot.set(xscale="log")
# plot.set(yscale="log")
# plot.set_xticklabels(rotation=-30)
# plot.set_axis_labels('n', 'time in ms')
# plot.set(title= "Chen_Instances")
#
# mp.savefig("plots/Chen_Instances.svg")
#
# mp.show()
#
# plot = sns.relplot(realTests[realTests["mode"] != "normal"],
#                    kind = 'scatter',
#                    x = 'n',
#                    y = 'runtime',
#                    hue = 'algorithm',
#                    palette = palette,
#                    style='mode',
#                    markers=markersMode,
#                    legend=True)
#
# plot.set(xscale="log")
# plot.set(yscale="log")
# plot.set_xticklabels(rotation=-30)
# plot.set_axis_labels('n', 'time in ms')
# plot.set(title= "Chen_Instances_Add_Delete")
#
# mp.savefig("plots/Chen_Instances_Add_Delete.svg")
#
# mp.show()