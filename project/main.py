import json
import os
import subprocess
from time import sleep

script_location = '/home/dietpi/rpi-rgb-led-matrix/examples-api-use/image-example'
image_location = '/home/dietpi/images/'
args_location = '/home/dietpi/src/matrix-settings.json'

jdata = open(args_location)
data = json.load(jdata)
args = []

for k,v in data.items():
	v = str(v)
	if v == '':
		args.append(k)
	else:
		args.append(k + '=' + v)

#args = ' '.join('='.join((k, str(v))) for (k, v) in data.items())
print(args)

visible_images = [f for f in os.listdir(image_location) if os.path.isfile(os.path.join(image_location, f))]
print(visible_images)

def run():
	i = 0
	cmd = [script_location] + args + [image_location + visible_images[i]]
	proc = subprocess.Popen(cmd)
	while True:
		print(cmd)
		sleep(10)
		if len(visible_images) > 1:
			proc.terminate()
			i += 1
			if i >= len(visible_images):	i = 0
			cmd[-1] = image_location + visible_images[i]
			proc = subprocess.Popen(cmd)

if __name__ == '__main__':
	run()
