

import time, os, random, pprint, sys, signal

def signal_handler(signal, frame):
    print "Ctrl+C caught.  Quitting... Jobs not registered as complete will be reran next time."
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

from multiprocessing import Process, Queue, current_process, freeze_support



jobs_file = "/Users/dan/tmp/jobs.txt"
completed_jobs_file = "/Users/dan/tmp/completed_jobs.txt"

symex = "symex"

NUMBER_OF_PROCESSES = 4

locally_completed_jobs = []




#
# Function run by worker processes
#

def worker(input, output):
	for func, args in iter(input.get, 'STOP'):
		result = execute_function(func, args)
		output.put(result)

# Hand-off function, allows diagnostics.
def execute_function(func, args):
	result = func(*args)

	# Verbose
	# print '%s says that %s%s = %s' % (current_process().name, func.__name__, args, result)
	return result
	


def get_jobs(jobs_to_read):
	jobs = []
	all_jobs = []
	completed_jobs = []
	all_completed_jobs = []

	# Read all the jobs with stripping
	with open(jobs_file, 'r') as f:
		all_jobs = [x.rstrip('\n').rstrip('\r').strip() for x in f.readlines()]

	# remove dupes but maintain order
	[jobs.append(item) for item in all_jobs if item not in jobs]

	# reopen completed jobs
	with open(completed_jobs_file, 'r') as f:
		all_completed_jobs = [x.rstrip('\n').rstrip('\r').strip() for x in f.readlines()]	

	# remove dupes
	[completed_jobs.append(item) for item in all_completed_jobs if item not in completed_jobs]

	# remove completed jobs from jobs list
	jobs = [x for x in jobs if not (x in completed_jobs or x in locally_completed_jobs)]

	# return requested amount of jobs
	return jobs[0:jobs_to_read]

def write_completed_job(job):
	with open(completed_jobs_file, 'a+') as f:
		f.write(job + "\n")

def execute_command(command, id):
	print str(id) + " | Running: " + command
	os.system(command + " ")
	print str(id) + " | Complete."
	return id
#
#
#

def run():

	if not os.path.isfile(jobs_file):
		print("Invalid jobs file.")

	if not os.path.exists(completed_jobs_file):
		print("Invalid completed jobs file.")

	# Create queues
	task_queue = Queue()
	done_queue = Queue()



	# Start worker processes
	for i in range(NUMBER_OF_PROCESSES):
		proc = Process(target=worker, args=(task_queue, done_queue))
		proc.daemon=True
		proc.start()

	active_tasks = {}
	# IDs aren't important now, but give options for later
	task_id = 0
	# Maximum iterations
	loops = 1
	# Or just run forever.
	forever = True


	while loops > 0 or forever:
		loops = loops - 1

		if(len(active_tasks) == 0 and len(get_jobs(NUMBER_OF_PROCESSES - len(active_tasks))) == 0):
			print "No work active and no work scheduled.  Sleeping for 1 minute."
			time.sleep(59)

		while not done_queue.empty():
			result = done_queue.get()

			if result in active_tasks.keys(): 
				# print "COMPLETED:" + active_tasks[result]
				write_completed_job(active_tasks[result])
				locally_completed_jobs.append(active_tasks[result])
				del active_tasks[result]
			else:
				print("ERROR!")
				print("Task completed, but never scheduled!")
				print("ID:" + str(result))
				sys.exit()




		if len(active_tasks) < NUMBER_OF_PROCESSES:
			jobs = get_jobs(NUMBER_OF_PROCESSES - len(active_tasks))

			for job in jobs:
				# print job, task_id
				task_queue.put((execute_command, (job, task_id)))
				active_tasks[task_id] = job

				task_id = task_id + 1



		time.sleep(1)



	# Tell child processes to stop
	for i in range(NUMBER_OF_PROCESSES):
		task_queue.put('STOP')


if __name__ == '__main__':
	if(len(sys.argv) >= 2):
		print sys.argv
		output_dir = os.path.abspath(sys.argv[1].strip().rstrip("/"))

		jobs_file = output_dir + "/jobs.txt"
		completed_jobs_file = output_dir + "/completed_jobs.txt"

		if not os.path.exists(output_dir):
			print "Creating directory: " + output_dir
			os.makedirs(output_dir)
		else:
			print "Directory found: " + output_dir

		existing_jobs = os.path.exists(jobs_file)

		if not existing_jobs:
			open(jobs_file, 'w+').close() 
			print "Creating jobs.txt"
		else:
			print "Existing jobs.txt found"

		if not os.path.exists(completed_jobs_file):
			open(completed_jobs_file, 'w+').close() 

		if(sys.argv[2].strip() == "start"):
			binary = os.path.abspath(sys.argv[3].strip())

			if not os.path.isfile(binary):
				print "Cannot find binary file: " + binary
				sys.exit()

			command = "symex " + binary + " --start-gps --output-dir " + output_dir

			print "Writing initial command: " + command
			with open(jobs_file, 'a+') as f:
				f.write(command + "\n") 
			# Job written!
		
		freeze_support()
		run()

	else:
		print "python " + sys.argv[0] +  " [output-dir] [start] [goto-binary]"

	sys.exit()
