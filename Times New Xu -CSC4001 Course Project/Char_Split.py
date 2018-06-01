import os
from PIL import Image, ImageFont, ImageDraw

A = open('mac.txt','r') 
a = A.readlines()
b = ''
for aa in a:
    b = b+aa
A.close()
  
b = b[:21]#b is your input text

im = Image.new("RGB", (400, 400), (255, 255, 255))
dr = ImageDraw.Draw(im)
if len(b) == 1:
    for bb in b:
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 250)
        dr.text((75,75), bb, font=font, fill="#000000")
elif len(b) == 2:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 150)
        dr.text((45+(i-1)*160, 125), bb, font=font, fill="#000000")
elif len(b) == 3:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 125)
        dr.text((25+(i-1)*120, 135), bb, font=font, fill="#000000")
elif len(b) == 4:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 150)
        if i < 3:
            dr.text((45+(i-1)*160, 40), bb, font=font, fill="#000000")
        else:
            dr.text((45+(i-3)*160, 205), bb, font=font, fill="#000000")
elif len(b) == 5:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 125)
        if i < 4:
            dr.text((25+(i-1)*120, 75), bb, font=font, fill="#000000")
        else:
            dr.text((75+(i-4)*130, 205), bb, font=font, fill="#000000")
elif len(b) == 6:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 125)
        if i < 4:
            dr.text((25+(i-1)*120, 75), bb, font=font, fill="#000000")
        else:
            dr.text((25+(i-4)*120, 205), bb, font=font, fill="#000000")
elif len(b) == 7:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 90)
        if i < 5:
            dr.text((20+(i-1)*90, 90), bb, font=font, fill="#000000")
        else:
            dr.text((60+(i-5)*95, 200), bb, font=font, fill="#000000")
elif len(b) == 8:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 90)
        if i < 5:
            dr.text((20+(i-1)*90, 90), bb, font=font, fill="#000000")
        else:
            dr.text((20+(i-5)*90, 200), bb, font=font, fill="#000000")
elif len(b) == 9:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 115)
        if i < 4:
            dr.text((25+(i-1)*120, 25), bb, font=font, fill="#000000")
        elif i < 7:
            dr.text((25+(i-4)*120, 145), bb, font=font, fill="#000000")
        else:
            dr.text((25+(i-7)*120, 270), bb, font=font, fill="#000000")
elif len(b) == 10:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 90)
        if i < 5:
            dr.text((20+(i-1)*90, 45), bb, font=font, fill="#000000")
        elif i < 8:
            dr.text((60+(i-5)*95, 150), bb, font=font, fill="#000000")
        else:
            dr.text((60+(i-8)*95, 255), bb, font=font, fill="#000000")
elif len(b) == 11:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 90)
        if i < 5:
            dr.text((20+(i-1)*90, 45), bb, font=font, fill="#000000")
        elif i < 9:
            dr.text((20+(i-5)*90, 150), bb, font=font, fill="#000000")
        else:
            dr.text((60+(i-9)*95, 255), bb, font=font, fill="#000000")
elif len(b) == 12:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 90)
        if i < 5:
            dr.text((20+(i-1)*90, 45), bb, font=font, fill="#000000")
        elif i < 9:
            dr.text((20+(i-5)*90, 150), bb, font=font, fill="#000000")
        else:
            dr.text((20+(i-9)*90, 255), bb, font=font, fill="#000000")
elif len(b) == 13:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 65), bb, font=font, fill="#000000")
        elif i < 10:
            dr.text((50+(i-6)*75, 150), bb, font=font, fill="#000000")
        else:
            dr.text((50+(i-10)*75, 235), bb, font=font, fill="#000000")
elif len(b) == 14:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 65), bb, font=font, fill="#000000")
        elif i < 11:
            dr.text((15+(i-6)*75, 150), bb, font=font, fill="#000000")
        else:
            dr.text((50+(i-11)*75, 235), bb, font=font, fill="#000000")
elif len(b) == 15:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 65), bb, font=font, fill="#000000")
        elif i < 11:
            dr.text((15+(i-6)*75, 150), bb, font=font, fill="#000000")
        else:
            dr.text((15+(i-11)*75, 235), bb, font=font, fill="#000000")
elif len(b) == 16:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 90)
        if i < 5:
            dr.text((20+(i-1)*90, 20), bb, font=font, fill="#000000")
        elif i < 9:
            dr.text((20+(i-5)*90, 110), bb, font=font, fill="#000000")
        elif i < 13:
            dr.text((20+(i-9)*90, 200), bb, font=font, fill="#000000")
        else:
            dr.text((20+(i-13)*90, 290), bb, font=font, fill="#000000")
elif len(b) == 17:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 20), bb, font=font, fill="#000000")
        elif i < 10:
            dr.text((50+(i-6)*75, 110), bb, font=font, fill="#000000")
        elif i < 14:
            dr.text((50+(i-10)*75, 200), bb, font=font, fill="#000000")
        else:
            dr.text((50+(i-14)*75, 290), bb, font=font, fill="#000000")
elif len(b) == 18:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 20), bb, font=font, fill="#000000")
        elif i < 11:
            dr.text((15+(i-6)*75, 110), bb, font=font, fill="#000000")
        elif i < 15:
            dr.text((50+(i-11)*75, 200), bb, font=font, fill="#000000")
        else:
            dr.text((50+(i-15)*75, 290), bb, font=font, fill="#000000")
elif len(b) == 19:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 20), bb, font=font, fill="#000000")
        elif i < 11:
            dr.text((15+(i-6)*75, 110), bb, font=font, fill="#000000")
        elif i < 16:
            dr.text((15+(i-11)*75, 200), bb, font=font, fill="#000000")
        else:
            dr.text((50+(i-16)*75, 290), bb, font=font, fill="#000000")
elif len(b) == 20:
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 20), bb, font=font, fill="#000000")
        elif i < 11:
            dr.text((15+(i-6)*75, 110), bb, font=font, fill="#000000")
        elif i < 16:
            dr.text((15+(i-11)*75, 200), bb, font=font, fill="#000000")
        else:
            dr.text((15+(i-16)*75, 290), bb, font=font, fill="#000000")
else:
    b = b[:20]
    i = 0
    for bb in b:
        i += 1
        font = ImageFont.truetype(os.path.join("simsun.ttc"), 75)
        if i < 6:
            dr.text((15+(i-1)*75, 20), bb, font=font, fill="#000000")
        elif i < 11:
            dr.text((15+(i-6)*75, 110), bb, font=font, fill="#000000")
        elif i < 16:
            dr.text((15+(i-11)*75, 200), bb, font=font, fill="#000000")
        else:
            dr.text((15+(i-16)*75, 290), bb, font=font, fill="#000000")
im.save("draw.png")
