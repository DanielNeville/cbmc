import glob, os, time, pprint, pickle, json, sys, subprocess, re, multiprocessing

from random import shuffle
from multiprocessing import Pool

bin_path = "/Users/dan/git/gps/src/bin/"

def system_call(command):
    print command
    return os.system(command)

def do_execution(command):
    full_command = timeout_string + command# + " > " + output_file + " 2>&1"
    # full_command = command
    # output_file_handle = open(output_file, 'w')
    # print(full_command)
    time_ = time.time()
    p=subprocess.Popen(full_command,shell=True)
    p.wait()
    # (out, err) = p.communicate()
    elapsed = time.time() - time_
    print elapsed
    return elapsed

def write_to_file(file_, data):
    # print file_
    tmp_f = open(str(file_), "w")
    tmp_f.write(data)
    tmp_f.close()

def worker(command):
    system_call(command)
    # print 'module name:', __name__
    # if hasattr(os, 'getppid'):  # only available on Unix
    #     print 'parent process:', os.getppid()
    # print 'process id:', os.getpid()

    return 1

if len(sys.argv) >= 2:
    final_commands = list()

    file = sys.argv[1]

    command = bin_path + "goto-gps " + file +  " --print-function-names --verbosity 0"
    directory = "py-runs/" + str(time.time())

    system_call("mkdir -p " + directory)

    system_call(command + " > " + directory + "/functions.txt") # Save for later.


    with open('functions.txt') as fp:
        for line in fp:
            final_commands.append("cbmc " + file + " --function " + line.strip() +
             " --cover exit --unwind 5 --trace --json-ui > " + directory + "/" + line.strip() + ".json")

    # #   pool = Pool(processes=)
    pool = Pool(processes=multiprocessing.cpu_count() - 1)

    pool.imap(worker, final_commands)

    pool.close()
    pool.join()

    print directory
