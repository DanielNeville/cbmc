

import time, os, random, pprint

from multiprocessing import Process, Queue, current_process, freeze_support

jobs_file = "/Users/dan/tmp/jobs.txt"
NUMBER_OF_PROCESSES = 4



if not os.path.isfile(jobs_file):
	print("Invalid jobs file.")
#
# Function run by worker processes
#

def worker(input, output):
	for func, args in iter(input.get, 'STOP'):
		result = execute_function(func, args)
		output.put(result)

#
# Function used to calculate result
#

def execute_function(func, args):
	result = func(*args)
	return '%s says that %s%s = %s' % \
		(current_process().name, func.__name__, args, result)


def get_jobs(lines_to_read):
	jobs = []
	with open(jobs_file, 'r') as f:
		while lines_to_read > 0:
			chunk = f.readline().strip()
			if chunk == "":
				break
			jobs.append(chunk)
			lines_to_read = lines_to_read - 1
	return jobs

def execute_command(command):
	print "Running: " + command
	os.system(command)

	return 0
#
#
#

def run():
	# TASKS1 = [(mul, (i, 7)) for i in range(20)]
	# TASKS2 = [(plus, (i, 8)) for i in range(10)]
	active_tasks = 0

	# Create queues
	task_queue = Queue()
	done_queue = Queue()

	# # Submit tasks
	# for task in TASKS1:
	#     task_queue.put(task)

	# Start worker processes
	for i in range(NUMBER_OF_PROCESSES):
		Process(target=worker, args=(task_queue, done_queue)).start()

	# # Get and print results
	# print 'Unordered results:'
	# for i in range(len(TASKS1)):
	#     print '\t', done_queue.get()

	# print "Adding more in 1..."



	# time.sleep(1)

	# print "Now!"

	# Add more tasks using `put()`
	# for task in TASKS2:
		# task_queue.put(task)

	jobs = get_jobs(NUMBER_OF_PROCESSES - active_tasks)

	for job in jobs:
		print job
		task_queue.put((execute_command, (job,)))
		active_tasks = active_tasks + 1


	# for job in jobs:
	# 	print job
	# 	task_queue.put((execute_command, (job,)))
	# 	active_tasks = active_tasks + 1
	


	# Get and print some more results
	# for i in range(len(TASKS2)):
	#     print '\t', done_queue.get()

	# Tell child processes to stop
	for i in range(NUMBER_OF_PROCESSES):
		task_queue.put('STOP')


if __name__ == '__main__':
	freeze_support()
	run()