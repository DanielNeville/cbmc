import os
import subprocess


jobs_file = "/Users/dan/tmp/jobs.txt"
processes = 4
max_queue_size = 10
active_processes = 0

class Queue:
    def __init__(self):
        self.items = []

    def empty(self):
        return self.items == []

    def add(self, item):
        self.items.insert(0,item)

    def remove(self):
        return self.items.pop()

    def size(self):
        return len(self.items)

    def output(self):
    	for i in reversed(self.items):
    		print i


if not os.path.isfile(jobs_file):
	print("Invalid jobs file.")

queue = Queue()

while True:
	# read jobs
	with open(jobs_file, 'r') as f:
		lines_to_read = max_queue_size - queue.size()
		while lines_to_read > 0:
			chunk = f.readline().strip()
			if chunk == "":
				break
			queue.add(chunk)
			lines_to_read = lines_to_read - 1

	while not queue.empty() and (processes - active_processes) > 0:
		next_job = queue.remove()
		active_processes = active_processes + 1
		print next_job

	break
