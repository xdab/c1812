import os
import PIL
import PIL.ImageDraw
import PIL.ImageFont
import PIL.Image

JOBFILE_CONTENT_FORMAT = """
# Radio parameters
freq 0.145
pol vertical
zone inland

# Geographical parameters
lat 52   
lon 21
n0 300
dn 40

# Transmitter parameters
txx 618588.1 
txy 507098.9
txh {0}
txpwr 5
txgain -4

# Receiver parameters 
#rxx 615614.8
#rxy 510614.89
rxh 2
rxgain 0

# Calculation parameters
p 50
radius 15000
xres 0.1
ares 1
threads 16

# Data files
terrain ./mazowieckie.txt.tf
clutter ./mazowieckie.tif.cf

# Output
out ./results.rf
"""

JOBFILE_NAME = 'tmp.job.txt'
FONT_PATH = '/usr/share/fonts/truetype/UbuntuMono-B.ttf'
FONT_SIZE = 32
FONT_FILL = (255, 255, 255)
FONT_BORDER = (0, 0, 0)
FONT_BORDER_WIDTH = 1
LABEL_POSITION = (28, 28)

H_MIN = 1.0
H_MAX = 25.0
H_STEP = 0.25

if __name__ == '__main__':
	font = PIL.ImageFont.truetype(FONT_PATH, FONT_SIZE)

	i = 0
	h = H_MIN
	frames = []
	while h <= H_MAX:
		with open(JOBFILE_NAME, 'w') as f:
			f.write(JOBFILE_CONTENT_FORMAT.format(h))

		os.system('./build/cli ' + JOBFILE_NAME)

		im = PIL.Image.open('image.bmp') \
			.convert('RGB') \
			.quantize(colors=10)
		
		imdraw = PIL.ImageDraw.Draw(im)
		imdraw.text(LABEL_POSITION, f"h = {h:5.2f} m", fill=FONT_FILL, stroke_fill=FONT_BORDER, stroke_width=FONT_BORDER_WIDTH, font=font)
		frames.append(im)

		print(f"Frame {i} (h={h:.3f}) done")

		i += 1
		h += H_STEP
		quit()

	FPS = 10
	frames[0].save('animation.gif', format='GIF', disposal=2, save_all=True, append_images=frames[1:],
				optimize=False, lossless=True, duration=1000 // FPS, loop=0)

	os.system(f'rm {JOBFILE_NAME}')
	os.system('rm results.rf')
