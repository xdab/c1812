import os
import PIL
import PIL.ImageDraw
import PIL.ImageFont
import PIL.Image
import subprocess

JOBFILE_CONTENT_FORMAT = """
freq 0.105
pol vertical
zone inland
lat 52   
lon 21
n0 300
dn 40
txx 618588.1 
txy 507098.9
txh {0}
txpwr 100
txgain 0
rxh 2
rxgain 0
p 50
radius 40000
xres 0.027
ares 2
threads 16
terrain ./mazowieckie.txt.tf
clutter ./mazowieckie.tif.cf
img ./{1}.bmp
"""

JOBFILE_NAME = 'tmp.job.txt'
FONT_PATH = '/usr/share/fonts/truetype/UbuntuMono-B.ttf'
FONT_SIZE = 32
FONT_FILL = (255, 255, 255)
FONT_BORDER = (0, 0, 0)
FONT_BORDER_WIDTH = 1
LABEL_POSITION = (28, 28)

H_MIN = 10
H_MAX = 100.0
H_STEP = 10

if __name__ == '__main__':
    font = PIL.ImageFont.truetype(FONT_PATH, FONT_SIZE)

    i = 0
    h = H_MIN
    processes = []

    hs = []
    while h <= H_MAX:
        hs.append(h)
        h += H_STEP

    for i, h in enumerate(hs):
        tmp_jobfile_name = f"{i}.{JOBFILE_NAME}"
        with open(tmp_jobfile_name, 'w') as f:
            f.write(JOBFILE_CONTENT_FORMAT.format(h, i))

    print("Jobfiles generated")

    for i, h in enumerate(hs):
        tmp_jobfile_name = f"{i}.{JOBFILE_NAME}"
        sp = subprocess.Popen(['./build/cli', tmp_jobfile_name])
        processes.append(sp)

    print("C1812 CLI subprocesses started")

    for i, sp in enumerate(processes):
        sp.wait()

    print("C1812 CLI subprocesses finished")

    frames = []
    for i, h in enumerate(hs):
        tmp_image_name = f"{i}.bmp"
        im = PIL.Image.open(tmp_image_name) \
                .convert('RGB') \
                .quantize(colors=10)
        imdraw = PIL.ImageDraw.Draw(im)
        imdraw.text(LABEL_POSITION, f"h = {h:5.2f} m", fill=FONT_FILL,
                    stroke_fill=FONT_BORDER, stroke_width=FONT_BORDER_WIDTH, font=font)
        frames.append(im)

    print("Frames generated")

    FPS = 10
    frames[0].save('animation.gif', format='GIF', disposal=2, save_all=True, append_images=frames[1:],
                   optimize=False, lossless=True, duration=1000 // FPS, loop=0)

    print("Animation saved")

    for i, h in enumerate(hs):
        tmp_jobfile_name = f"{i}.{JOBFILE_NAME}"
        tmp_image_name = f"{i}.bmp"
        os.system(f'rm {tmp_jobfile_name} {tmp_image_name}')

    print("Temporary files removed")
    print("Done")
