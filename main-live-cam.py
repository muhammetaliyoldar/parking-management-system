import cv2
import numpy as np
import urllib.request
from ocr import process_plate
from database import init_db
import time


def live_camera_processing():
    """
    ESP32-CAM üzerinden anlık plaka algılama ve görüntü gösterimi.
    """
    # ESP32-CAM'in IP adresi ve stream URL'i
    esp32_cam_url = "http://192.168.123.200/stream"  # IP adresinizi güncelleyin

    print("ESP32-CAM bağlantısı başlatılıyor...")

    last_plate_time = None  # Son plaka işleme zamanı
    last_detected_plate = None  # Son algılanan plaka
    bytes_data = bytes()

    while True:
        try:
            # ESP32-CAM'den görüntü akışı al
            stream = urllib.request.urlopen(esp32_cam_url)
            while True:
                bytes_data += stream.read(1024)
                a = bytes_data.find(b'\xff\xd8')  # JPEG başlangıcı
                b = bytes_data.find(b'\xff\xd9')  # JPEG sonu
                if a != -1 and b != -1:
                    jpg = bytes_data[a:b+2]
                    bytes_data = bytes_data[b+2:]
                    frame = cv2.imdecode(np.frombuffer(jpg, np.uint8), cv2.IMREAD_COLOR)

                    if frame is None:
                        print("Görüntü alınamadı!")
                        continue

                    current_time = time.time()

                    # Eğer 30 saniye geçmediyse işlem yapma
                    if last_plate_time and (current_time - last_plate_time < 30):
                        cv2.imshow("Plaka Algılama - ESP32-CAM", frame)
                        if cv2.waitKey(1) & 0xFF == ord('q'):
                            print("Çıkış yapılıyor...")
                            return
                        continue

                    # Plaka algılama ve işleme
                    plates = process_plate(frame)
                    if plates:
                        for plate in plates:
                            if plate != last_detected_plate:
                                print(f"Plaka algılandı: {plate}")
                                handle_vehicle_entry_exit(plate)  # Giriş/çıkış işlemi yap
                                last_detected_plate = plate
                                last_plate_time = current_time  # Son işlem zamanını güncelle
                                break
                    else:
                        print("Plaka algılanamadı.")

                    # Görüntüyü ekranda göster
                    cv2.imshow("Plaka Algılama - ESP32-CAM", frame)

                    # Çıkış için 'q' tuşuna basın
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        print("Çıkış yapılıyor...")
                        return

        except Exception as e:
            print(f"Hata: {e}")
            time.sleep(2)  # Hata durumunda 2 saniye bekle
            continue

    cv2.destroyAllWindows()


if __name__ == "__main__":
    init_db()  # Veritabanını başlat
    live_camera_processing()
