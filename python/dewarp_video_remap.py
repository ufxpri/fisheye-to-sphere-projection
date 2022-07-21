import matplotlib.pyplot as plt
from math import sin, cos, acos, tan, atan, atan2, pi, sqrt, pow

import cv2
import numpy as np

cap = cv2.VideoCapture("43.mp4")
_, frame = cap.read()
frame = cv2.resize(frame, None, fx=1, fy=1, interpolation=cv2.INTER_LINEAR)
frame_width = len(frame[0])
frame_height = len(frame)
remap_width = 200
remap_height = 200


#===========================================================================================
circle_center = (int(frame_width/2), int(frame_height/2))
circle_radius = int(frame_height/2)

drag_enable = False
def mouse_event(event, x, y, flags, param):
    global circle_center, circle_radius, drag_enable
    if event == cv2.EVENT_LBUTTONDOWN:
        drag_enable = True
    if event == cv2.EVENT_LBUTTONUP:
        drag_enable = False
    if event == cv2.EVENT_MOUSEMOVE:
        if drag_enable:
            circle_center = (x, y)
    if event == cv2.EVENT_MOUSEWHEEL:
        if flags > 0:
            circle_radius += 1
        elif circle_radius > 0:
                circle_radius -= 1
            
cv2.namedWindow('draw')
cv2.setMouseCallback('draw', mouse_event)
while True:
    canvas = frame.copy()
    cv2.circle(canvas, circle_center, circle_radius, (255,0,0), 1)
    cv2.imshow('draw', canvas)
    key = cv2.waitKey(1)
    if key == ord('a'): circle_center= (circle_center[0] - 1, circle_center[1])
    elif key == ord('d'): circle_center= (circle_center[0] + 1, circle_center[1])
    elif key == ord('w'): circle_center= (circle_center[0], circle_center[1] - 1)
    elif key == ord('s'): circle_center= (circle_center[0], circle_center[1] + 1)
    elif key == ord('q') or key == ord('\n'): break
            
cv2.destroyAllWindows()
print('circle seting done !!')
print('input frame size: ', len(frame), len(frame[0]))
#===========================================================================================

# view coordinate related
view_theta = 0 # 0 <= theta <= 2*pi
view_phi = 0 # 0 <= phi <= pi/2       0 is bottom

# view area related
view_angle = 2*pi * 1/3 # 
view_height = view_angle * remap_height/remap_width

map_x = np.zeros((remap_height,remap_width,1), np.float32)
map_y = np.zeros((remap_height,remap_width,1), np.float32)

def make_map(arg):
    global map_x, map_y
    global view_theta, view_phi, view_angle, view_height

    mask = frame.copy()

    theta = cv2.getTrackbarPos('theta','remap') /100
    phi = cv2.getTrackbarPos('phi','remap') /100
    angle = cv2.getTrackbarPos('angle','remap') /100

    view_angle = pi* angle 
    view_height = view_angle * remap_height/remap_width
    view_theta = 2*pi* theta
    view_phi = (pi/2 - view_angle/2)* phi

    aXr = view_angle / remap_width
    bXr = view_angle/2
    aYr = view_height / remap_height
    bYr = view_height/2

    sin_view_phi = sin(-view_phi)
    cos_view_phi = cos(-view_phi)
    sin_view_theta = sin(-view_theta)
    cos_view_theta = cos(-view_theta)
    
    for x in range(remap_width):
        for y in range(remap_height):
            # flat 이미지에서 좌표값에 따른 회전각 (왼쪽위 = +Xr -Yr 오른쪽아래 = -Xr +Yr)
            Xr = x * aXr - bXr # -Xr ~  Xr
            Yr = y * aYr - bYr # -Yr ~  Yr

            # 회전각에 따른 view에서의 벡터값 (중심축은 view 벡터, 그로부터 벌어진 각도가 Xr Xy)
            Vx = -tan(Yr)
            Vy = tan(Xr)

            # view로부터의 백터값 V 를 view 벡터에 맞도록 변환 (y_rotate -> z_rotate) -> 구면에서의 벡터값
            # V = y_rotate(V, -view_phi)
            # V = z_rotate(V, -view_theta)  
            Vx, Vy, Vz = cos_view_phi*Vx-sin_view_phi, Vy, -cos_view_phi-sin_view_phi*Vx
            Vx, Vy, Vz = cos_view_theta*Vx-sin_view_theta*Vy, sin_view_theta*Vx+cos_view_theta*Vy, Vz

            # 벡터의 세타 파이값 추출
            V_theta = atan2(Vy,Vx)
            V_phi = atan2(sqrt(Vx*Vx + Vy*Vy), (-Vz))

            r = (V_phi / (pi/2)) * (circle_radius)
            Cx = r * cos(V_theta)
            Cy = r * sin(V_theta)

            # # x, y 좌표값
            map_x[y][x] = Cx
            map_y[y][x] = Cy
            
            mask[int(Cy + circle_center[1])][int(Cx + circle_center[0])] = ((Xr+view_angle/2)/view_angle*255,0,(Yr+view_height/2)/view_height*255) # B G R
    
    # 구면 이미지에서의 x, y 좌표값
    map_x = map_x + circle_center[0]
    map_y = map_y + circle_center[1]

    mask = cv2.resize(mask, None, fx=0.2, fy=0.2, interpolation=cv2.INTER_LINEAR)
    cv2.imshow('mask', mask)
    cv2.waitKey(1)


if __name__ == "__main__":
    cv2.namedWindow('remap')
    cv2.createTrackbar('theta','remap',0,100,make_map)
    cv2.createTrackbar('phi','remap',0,100,make_map)
    cv2.createTrackbar('angle','remap',30,100,make_map)

    i = 0
    while True:
        ret, frame = cap.read()
        if not ret: break
        frame = cv2.resize(frame, None, fx=1, fy=1, interpolation=cv2.INTER_LINEAR)
        
        remap = np.zeros((remap_height,remap_width,3), np.uint8)

        remap = cv2.remap(frame, map_x, map_y, cv2.INTER_LINEAR)
        cv2.imshow('remap', remap)

        if cv2.waitKey(1) == ord('q'): break
        print(i)
        i += 1
