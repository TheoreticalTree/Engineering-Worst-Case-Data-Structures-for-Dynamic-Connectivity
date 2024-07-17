import simexpal

def parse(run):
    with run.open_output_file() as f:
        return f.readline()


results_regular = []
results_MaxUpdate = []
results_queryTime = []
results_accuracy = []

cfg = simexpal.config_for_dir()
for successful_run in cfg.collect_successful_results():
    result = parse(successful_run)
    if "regular" in result:
        results_regular.append(result)
    elif "maxTimeUpdate" in result:
        results_MaxUpdate.append(result)
    elif "queryTime" in result:
        results_queryTime.append(result)
    elif "correctnessCheck" in result:
        results_accuracy.append(result)

with open("results/experiments_regular.csv", "w") as csv_regular:
    print("mode,algorithm,n,density,queryFrequency,instance,runtime", file=csv_regular, end='\n')
    for result in results_regular:
        print(result.lstrip(), file=csv_regular, end='')

with open("results/experiments_maxTimeUpdate.csv", "w") as csv_maxTimeUpdate:
    print("mode,algorithm,n,density,queryFrequency,instance,maxTimeInsert,maxTimeDelete", file=csv_maxTimeUpdate, end='\n')
    for result in results_MaxUpdate:
        print(result.lstrip(), file=csv_maxTimeUpdate, end='')

with open("results/experiments_queryTime.csv", "w") as csv_queryTime:
    print("mode,algorithm,n,density,queryFrequency,instance,queryTime", file=csv_queryTime, end='\n')
    for result in results_queryTime:
        print(result.lstrip(), file=csv_queryTime, end='')

with open("results/experiments_correctnessCheck.csv", "w") as csv_correctnessCheck:
    print("mode,algorithm,n,density,queryFrequency,instance,checks,errorStates,queries,queryErrors", file=csv_correctnessCheck, end='\n')
    for result in results_accuracy:
        print(result.lstrip(), file=csv_correctnessCheck, end='')