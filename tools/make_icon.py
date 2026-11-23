#!/usr/bin/env python3
from PIL import Image, ImageDraw, ImageFont
S=512
im=Image.new("RGB",(S,S),(7,13,24))
p=im.load()
for y in range(S):
 for x in range(S):
  t=(x+y)/(2*S)
  p[x,y]=(int(7+7*t),int(20+28*t),int(36+52*t))
d=ImageDraw.Draw(im)
for x in range(0,S,32): d.line((x,0,x,S),fill=(16,39,60),width=1)
for y in range(0,S,32): d.line((0,y,S,y),fill=(16,39,60),width=1)
d.rounded_rectangle((38,38,474,474),radius=46,outline=(43,178,255),width=8)
d.rounded_rectangle((78,92,434,420),radius=30,fill=(10,25,42),outline=(42,78,108),width=3)
f=ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",150)
b=d.textbbox((0,0),"P",font=f)
d.text(((S-(b[2]-b[0]))/2,118),"P",font=f,fill=(238,247,255),stroke_width=2,stroke_fill=(43,178,255))
f2=ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",36)
d.text((100,322),"PAY-LAUNCH",font=f2,fill=(92,202,255))
im.save("sce_sys/icon0.png")