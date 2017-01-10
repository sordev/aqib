# АЧИЛ буюу Агаарын Чанарын Индекс тодорхойлогч Лонх 
(English Readers Please See Below)
АЧИЛ нь агаарын чанарыг тодорхойлох төхөөрөмж бөгөөд хамгийн гол нь хямд зардлаар боломжийн агаарын чанар тодорхойлох төхөөрөмж бий болгохын тулд хийгдсэн. Энэхүү код нь нээлттэй эхийнх ба бусад төрөл бүрийн нээлттэй эхээс жишээ авч бүтээгдсэн болно.

### АЧИЛ нь дараах зүйлсийг тодорхойлно.
  - PM2.5 болон PM10 тоосонцор.
  - SO2, O3, NO2, CO, NH3 химийн нэгдлүүд
  - Орчны температур, чийгшил

### АЧИЛ -н боломжууд.
  - WIFI модуль ашиглан интернэтээр дамжуулан мэдээллээ сервер дээр хадгалах

### Шаардлагатай сэлбэгүүд.
https://docs.google.com/spreadsheets/d/11MSpzdKqcEXNcD25FfU-qca580eOnAqrInOi8nA_Huc/edit?usp=sharing

### АЧИЛ -г бүтээх.
Дараах линкээр орж хэрхэн хийх талаар дэлгэрэнгүй мэдээлэл аваарай.

### Суулгац
Ардиуно https://www.arduino.cc/en/Main/Software -г татан авч ашиглана. Ашиглах сангууд
- Wire.h
- SPI.h
- LiquidCrystal.h
- DHT

### Хэрэглээ
- utaa.mn сайттай холбогдож өөрийн төхөөржийг бүртгүүлж нэвтрэх түлхүүр авна.
- Ардиуно руу байршуулахын өмнө өөрийн ВайФай -н холболтын мэдээлэл болон utaa.mn -с авсан түлхүүрийг оруулж өгнө. Холболт амжилттай хийгдвэл дэлгэц дээр мэдээлэл гарна.
- Төхөөрөмж амжилттай ажиллаж байвал дэлгэц дээр тоон үзүүлэлтүүд харагдах ба харагдаж байгаа тоон үзүүлэлтүүдийг сервер лүү автоматаар илгээж эхлэнэ. Төхөөрөмж нь эхний 24 - 48 цаг сенсор шатаалт явагдах бөгөөд түүнээс хойш үнэн бодит заах магадлалтай болно.

# AQIB Air Quality Index Bottle
AQIB is cheap solution for AQI sensor. Based on arduino and various sensors.

# AQIB can sense following
- PM2.5 and PM10 
- SO2, O3, NO2, CO and NH3 chemicals
- Temperature and humidity

# AQIB Features
- Will collect data and send to main server

# Required PARTS
https://docs.google.com/spreadsheets/d/11MSpzdKqcEXNcD25FfU-qca580eOnAqrInOi8nA_Huc/edit?usp=sharing

# How to build?

# Install
Visit https://www.arduino.cc/en/Main/Software for software. Clone code to your local. Install/Add following librarires
- Wire.h
- SPI.h
- LiquidCrystal.h
- DHT
Compline and upload to Arduino via software.

# Usage
- Ask utaa.mn for API Key
- Change variables inside code with your WiFi connection username/password, Update API Key with your own Key which is provided by utaa.mn
- If AQIB working and connected to WiFi you'll see information on screen, After succesful connection AQIB will show data on screen and will start sending data to server. Sensors requires to be burned out for at least 24-48 hours for good result.
