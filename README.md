# ProneSystem

Developed with Unreal Engine 4

## 목적         
<img src="ProneImage.png" width="700">
 좀 더 현실적인 엎드리기 시스템 구현을 목적으로 만든 저장소 입니다.
목표는 맥스페인과 메탈기어 솔리드와 같이 엎드린 상태에서 360도 회전을 할 때 자연스러운 모습을 보이게 만드는 것입니다.

## 구현방법(기본 이론)
<img src="ProneImage2.png" width="500">

 기본 이론은 Actor Rotation을 돌린 만큼(Controller Rotation) Skeletal Mesh의 Root Rotation을 반대로 돌리면 
Actor Rotation은 돌지만 외형은 가만히 있는 것 처럼 보이게 됩니다.

<img src="ProneImage3.png" width="700">
 엎드리기 상태에 들어가기 시작할때 Actor Rotation을 구해서 TurnDir에 넣고 (TurnDir이 중심점이 됩니다.) 
플레이어의 Controller Rotation에 따라 Actor Rotation이 움직이면
TurnDir과 Actor Rotation의 각도(Yaw)의 차이만큼 Skeletal Mesh의 Root Rotation을 반대로 돌리면 됩니다.

## 영상

{% include video id="nY_vBttBVgw" provider="youtube" %}

## 진행상황
360도 회전 구현 완료, 이동시 중심축 이동, 뒤집혀진 상태(등이 바닥을 향하는)에서 일어서기 구현
