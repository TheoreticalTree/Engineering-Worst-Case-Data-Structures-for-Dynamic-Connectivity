import random

from typing import List, Tuple


def generateRandomClusterGraph(n: int, cliqueSize: int, numSteps: int, seed: int, outFile: str, outFileBQ: str):
    vertices: List[int] = [1] * n
    for i in range(0, n):
        vertices[i] = i

    random.seed(seed)

    random.shuffle(vertices)

    cliqueInternalEdges: List[Tuple[int, int]] = []
    interCliqueEdges: List[Tuple[int, int]] = []

    #We want roughly 50% of all edges in the cliques to be in use
    for i in range(0, int(n / cliqueSize)):
        for j in range(0, cliqueSize):
            for k in range(j + 1, cliqueSize):
                if random.randint(0, 1) == 1:
                    cliqueInternalEdges.append((vertices[i * cliqueSize + j], vertices[i * cliqueSize + k]))

    #We want roughly 4c edges between cliques, where c is the number of cliques
    # No two cliques are supposed to be connected more then once
    for i in range(0, int(n / cliqueSize)):
        for j in range(i + 1, int(n / cliqueSize)):
            if random.randint(0, int(n / cliqueSize)) <= 8:
                u: int = vertices[cliqueSize * i + random.randint(0, cliqueSize - 1)]
                v: int = vertices[cliqueSize * j + random.randint(0, cliqueSize - 1)]
                interCliqueEdges.append((u, v))

    random.shuffle(interCliqueEdges)
    random.shuffle(cliqueInternalEdges)

    borderInOutInterCliques: int = int(len(interCliqueEdges) / 2)
    borderInOutInsideCliques: int = int(len(cliqueInternalEdges) / 2)

    with open(outFile, "w") as output:
        with open(outFileBQ, "w") as outputBQ:
            output.write("c file: " + outFile + " " + "0" + "\n")
            output.write("c cliqueTest: " + str(cliqueSize) + "\n")
            outputBQ.write("c file: " + outFile + " " + "0" + "\n")
            outputBQ.write("c cliqueTest: " + str(cliqueSize) + "\n")


            for i in range(0, borderInOutInterCliques):
                output.write("a " + str(interCliqueEdges[i][0]) + " " + str(interCliqueEdges[i][1]) + "\n")
                outputBQ.write("a " + str(interCliqueEdges[i][0]) + " " + str(interCliqueEdges[i][1]) + "\n")

            for i in range(0, borderInOutInsideCliques):
                output.write("a " + str(cliqueInternalEdges[i][0]) + " " + str(cliqueInternalEdges[i][1]) + "\n")
                outputBQ.write("a " + str(cliqueInternalEdges[i][0]) + " " + str(cliqueInternalEdges[i][1]) + "\n")

            for i in range(0, numSteps + 1000):
                if i == 1000:
                    output.write("t 0 0\n")
                    outputBQ.write("t 0 0\n")
                if (i - 1000) % int(numSteps / 100) == 0:
                    outputBQ.write("b\n")
                if random.randint(0, 1) == 0:
                    j: int = random.randint(0, len(interCliqueEdges) - 1)
                    if j < borderInOutInterCliques:
                        (interCliqueEdges[j], interCliqueEdges[borderInOutInterCliques - 1]) = (interCliqueEdges[borderInOutInterCliques - 1], interCliqueEdges[j])
                        output.write("d " + str(interCliqueEdges[borderInOutInterCliques - 1][0]) + " " + str(interCliqueEdges[borderInOutInterCliques - 1][1]) + "\n")
                        outputBQ.write("d " + str(interCliqueEdges[borderInOutInterCliques - 1][0]) + " " + str(interCliqueEdges[borderInOutInterCliques - 1][1]) + "\n")
                        borderInOutInterCliques -= 1
                    else:
                        (interCliqueEdges[j], interCliqueEdges[borderInOutInterCliques]) = (interCliqueEdges[borderInOutInterCliques], interCliqueEdges[j])
                        output.write("a " + str(interCliqueEdges[borderInOutInterCliques][0]) + " " + str(interCliqueEdges[borderInOutInterCliques][1]) + "\n")
                        outputBQ.write("a " + str(interCliqueEdges[borderInOutInterCliques][0]) + " " + str(interCliqueEdges[borderInOutInterCliques][1]) + "\n")
                        borderInOutInterCliques += 1
                else:
                    j: int = random.randint(0, len(cliqueInternalEdges) - 1)
                    if j < borderInOutInsideCliques:
                        (cliqueInternalEdges[j], cliqueInternalEdges[borderInOutInsideCliques - 1]) = (cliqueInternalEdges[borderInOutInsideCliques - 1], cliqueInternalEdges[j])
                        output.write("d " + str(cliqueInternalEdges[borderInOutInsideCliques - 1][0]) + " " + str(cliqueInternalEdges[borderInOutInsideCliques - 1][1]) + "\n")
                        outputBQ.write("d " + str(cliqueInternalEdges[borderInOutInsideCliques - 1][0]) + " " + str(cliqueInternalEdges[borderInOutInsideCliques - 1][1]) + "\n")
                        borderInOutInsideCliques -= 1
                    else:
                        (cliqueInternalEdges[j], cliqueInternalEdges[borderInOutInsideCliques]) = (cliqueInternalEdges[borderInOutInsideCliques], cliqueInternalEdges[j])
                        output.write("a " + str(cliqueInternalEdges[borderInOutInsideCliques][0]) + " " + str(cliqueInternalEdges[borderInOutInsideCliques][1]) + "\n")
                        outputBQ.write("a " + str(cliqueInternalEdges[borderInOutInsideCliques][0]) + " " + str(cliqueInternalEdges[borderInOutInsideCliques][1]) + "\n")
                        borderInOutInsideCliques += 1