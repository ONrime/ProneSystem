# ProneSystem

Developed with Unreal Engine 4

## 목적         
<img src="ProneImage.png" width="700">
좀 더 현실적인 엎드리기 시스템 구현을 목적으로 만든 저장소 입니다.
목표는 맥스페인과 메탈기어 솔리드와 같이 엎드린 상태에서 360도 회전을 할 때 자연스러운 모습을 보이게 만드는 것입니다.

## 구현방법
<img src="ProneImage2.png" width="700">
Actor Rotation을 돌린 만큼(Controller Rotation) Skeletal Mesh의 Root Rotation을 반대로 돌리면 Actor Rotation은 돌지만 외형은 가만히 있는 것 처럼 보이게 됩니다.
