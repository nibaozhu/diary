import time
import win32gui, win32ui, win32con, win32api
def window_capture(filename):
  hwnd = 0 # ���ڵı�ţ�0�ű�ʾ��ǰ��Ծ����
  # ���ݴ��ھ����ȡ���ڵ��豸������DC��Divice Context��
  hwndDC = win32gui.GetWindowDC(hwnd)
  # ���ݴ��ڵ�DC��ȡmfcDC
  mfcDC = win32ui.CreateDCFromHandle(hwndDC)
  # mfcDC�����ɼ��ݵ�DC
  saveDC = mfcDC.CreateCompatibleDC()
  # ����bigmap׼������ͼƬ
  saveBitMap = win32ui.CreateBitmap()
  # ��ȡ�������Ϣ
  MoniterDev = win32api.EnumDisplayMonitors(None, None)
  w = MoniterDev[0][2][2]
  h = MoniterDev[0][2][3]
  # print w,h������#ͼƬ��С
  # Ϊbitmap���ٿռ�
  saveBitMap.CreateCompatibleBitmap(mfcDC, w, h)
  # �߶�saveDC������ͼ���浽saveBitmap��
  saveDC.SelectObject(saveBitMap)
  # ��ȡ�����Ͻǣ�0��0������Ϊ��w��h����ͼƬ
  saveDC.BitBlt((0, 0), (w, h), mfcDC, (0, 0), win32con.SRCCOPY)
  saveBitMap.SaveBitmapFile(saveDC, filename)
beg = time.time()
for i in range(10):
  window_capture("haha.jpg")
end = time.time()
print(end - beg)
