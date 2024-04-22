import socket
import cv2
import mediapipe as mp
import json

# MediaPipe 핸드 모델 및 드로잉 유틸리티 초기화
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(min_detection_confidence=0.5, min_tracking_confidence=0.5)
mp_draw = mp.solutions.drawing_utils

# 소켓 설정
host = '127.0.0.1'  # 서버의 IP 주소
port = 65431        # 서버의 포트
socket_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket_server.bind((host, port))  # 호스트와 포트 바인딩
socket_server.listen()            # 클라이언트의 연결 기다림
print(f"서버가 {host}:{port}에서 연결을 기다립니다.")

conn, addr = socket_server.accept()  # 연결 허용
print(f"연결된 클라이언트 주소: {addr}")

# 웹캠에서 영상을 가져오기
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

try:
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            continue

        # BGR 이미지를 RGB로 변환
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = hands.process(image_rgb)

        # 모든 손에 대한 데이터를 담을 딕셔너리 초기화
        hands_data = {'hands': []}

        if results.multi_hand_landmarks:
            for hand_landmarks, handedness in zip(results.multi_hand_landmarks, results.multi_handedness):
                # 원래의 손 종류를 가져오고, 강제로 반대로 설정
                original_hand_type = handedness.classification[0].label  # 'Right' 또는 'Left'
                hand_type = "Left" if original_hand_type == "Right" else "Right"

                mp_draw.draw_landmarks(image, hand_landmarks, mp_hands.HAND_CONNECTIONS)

                # 각 손에 대한 랜드마크 정보 저장
                landmarks = [{'id': id, 'x': int(lm.x * image.shape[1]), 'y': int(lm.y * image.shape[0]), 'z': lm.z} for id, lm in enumerate(hand_landmarks.landmark)]
                hands_data['hands'].append({'type': hand_type, 'landmarks': landmarks})

            # 손의 데이터를 JSON 형식으로 변환하여 전송
            data_json = json.dumps(hands_data)
            conn.sendall(data_json.encode('utf-8'))

        cv2.imshow('Hand Tracking', image)
        if cv2.waitKey(5) & 0xFF == 27:
            break

finally:
    cap.release()
    cv2.destroyAllWindows()
    conn.close()
    socket_server.close()

    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
