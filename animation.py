import os
import PIL
import PIL.ImageDraw
import PIL.ImageFont
import PIL.Image
import subprocess

JOBFILE_CONTENT_FORMAT = """
frequency 0.145
polarization vertical
zone inland
latitude 52   
longitude 21
n0 300
dn 40
tx_x 618590
tx_y 507100
tx_h {0}
tx_power 5
tx_gain -4
rx_h 2
rx_gain -4
p 50
radius 20000
spatial_resolution 0.03
angular_resolution 2
threads 8
data_terrain ./mazowieckie.txt.tf
data_clutter ./mazowieckie.tif.cf
out_img ./{1}.bmp
out_img_size 500
out_img_colormap coolwarm
out_img_scale_min 1
out_img_scale_max 9
out_img_data_type s-units
"""

JOBFILE_NAME = 'tmp.job.txt'
FONT_PATH = '/usr/share/fonts/truetype/UbuntuMono-B.ttf'
FONT_SIZE = 32
FONT_FILL = (255, 255, 255)
FONT_BORDER = (0, 0, 0)
FONT_BORDER_WIDTH = 1
LABEL_POSITION = (28, 28)
COLORS = 255

H_MIN = 2.0
H_MAX = 20.0
H_STEP = 0.25

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
    first_frame = None
    for i, h in enumerate(hs):
        tmp_image_name = f"{i}.bmp"

        im = PIL.Image.open(tmp_image_name)
        if first_frame is None:
            im = im.quantize(colors=COLORS)
            first_frame = im.copy()
        else:
            im = im.quantize(colors=COLORS, palette=first_frame)
        
        imdraw = PIL.ImageDraw.Draw(im)
        imdraw.text(LABEL_POSITION, f"h = {h:5.2f} m", fill=FONT_FILL,
                    stroke_fill=FONT_BORDER, stroke_width=FONT_BORDER_WIDTH, font=font)
        frames.append(im)

    print("Frames generated")

    FPS = 10
    frames[0].save('animation.gif', format='GIF', save_all=True, append_images=frames[1:],
                   optimize=True, lossless=True, duration=1000 // FPS, loop=0)

    print("Animation saved")

    for i, h in enumerate(hs):
        tmp_jobfile_name = f"{i}.{JOBFILE_NAME}"
        tmp_image_name = f"{i}.bmp"
        os.system(f'rm {tmp_jobfile_name} {tmp_image_name}')

    print("Temporary files removed")
    print("Done")
