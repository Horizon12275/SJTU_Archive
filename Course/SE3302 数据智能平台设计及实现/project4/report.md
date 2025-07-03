```bash
10.119.15.62

./clash-linux-amd64-v1.10.0 -f glados.yaml -d .
nohup ./clash-linux-amd64-v1.10.0 -f glados.yaml -d . > clash.log 2>&1 &

docker pull ghcr.io/home-assistant/home-assistant:stable

docker run -d \
    --name homeassistant \
    --restart=unless-stopped \
    -e TZ=Asia/Shanghai \
    -v /home/username/homeassistant:/config \
    --network=host \
    ghcr.io/home-assistant/home-assistant:stable

cd /home/username/homeassistant
git clone https://github.com/XiaoMi/ha_xiaomi_home.git
cd ha_xiaomi_home
chmod +x ./install.sh
./install.sh ../
docker restart homeassistant

git clone https://github.com/espressif/esp-adf.git
git submodule update --init --recursive

idf.py set-targer esp32s3
```
