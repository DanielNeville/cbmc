

import time, os, random, pprint

from multiprocessing import Process, Queue, current_process, freeze_support

jobs_file = "/Users/dan/tmp/jobs.txt"
completed_jobs_file = "/Users/dan/tmp/completed_jobs.txt"

NUMBER_OF_PROCESSES = 1

if not os.path.exists(completed_jobs_file):
    open(completed_jobs_file, 'w+').close() 

locally_completed_jobs = []


if not os.path.isfile(jobs_file):
	print("Invalid jobs file.")

if not os.path.exists(completed_jobs_file):
	print("Invalid completed jobs file.")
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

	print '%s says that %s%s = %s' % \
		(current_process().name, func.__name__, args, result)
	return result
	


def get_jobs(jobs_to_read):
	print "GETTING JOBS!"


	jobs = []
	completed_jobs = []

	with open(jobs_file, 'r') as f:
		all_jobs = [x.rstrip('\n').rstrip('\r').strip() for x in f.readlines()]

	jobs = []
	[jobs.append(item) for item in all_jobs if item not in jobs]

	with open(completed_jobs_file, 'r') as f:
		completed_jobs = [x.rstrip('\n').rstrip('\r').strip() for x in f.readlines()]		

	i = 0

	for job in jobs:
		print str(i) + job
		i = i + 1
		if job in completed_jobs or job in locally_completed_jobs:
			print "REMOVED"
			jobs.remove(job)

		if(job == "cbmc bug.o --cover exit --function _start"):
			print "HELLO"

		print (job in completed_jobs)


	print "completed"

	i = 0

	for job in completed_jobs:
		print str(i) + job
		i = i + 1		

		if(job== "cbmc bug.c --cover exit --function _start"):
			print "HELLO2"

	return jobs[0:jobs_to_read]


	# with open(jobs_file, 'r') as f:
	# 	while lines_to_read > 0:
	# 		chunk = f.readline().strip()
	# 		if chunk == "":
	# 			break
	# 		jobs.append(chunk)
	# 		lines_to_read = lines_to_read - 1
	# return jobs

def write_completed_job(job):
	with open(completed_jobs_file, 'a+') as f:
		f.write(job + "\n")

def execute_command(command, id):
	print "Running: " + command
	os.system(command)

	return id
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
		proc = Process(target=worker, args=(task_queue, done_queue))
		proc.daemon=True
		proc.start()

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

	active_tasks = {}

	task_id = 0

	loops = 2

	while loops > 0:
		loops = loops - 1

		while not done_queue.empty():
			result = done_queue.get()

			if result in active_tasks.keys(): 
				print "COMPLETED:" + active_tasks[result]
				write_completed_job(active_tasks[result])
				locally_completed_jobs.append(active_tasks[result])
				del active_tasks[result]
			else:
				print("ERROR!")




		if len(active_tasks) < NUMBER_OF_PROCESSES:
			jobs = get_jobs(NUMBER_OF_PROCESSES - len(active_tasks))

			for job in jobs:
				# print job, task_id
				task_queue.put((execute_command, (job, task_id)))
				active_tasks[task_id] = job

				task_id = task_id + 1

		print active_tasks
		

		time.sleep(2)

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