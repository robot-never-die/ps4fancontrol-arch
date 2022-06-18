# PS4 Fan Control for ArchLinux
A program for adjust PS4 Linux threshold temperature.

---
## Build and Install
```bash
sudo pacman -S base-devel
git clone https://github.com/robot-never-die/ps4fancontrol-arch
cd ps4fancontrol-arch
make
sudo mv ps4fancontrol /usr/bin
```
## Usage
Set threshold 60°C
```bash
sudo ps4fancontrol --threshold 60
```
## Service
```bash
sudo mv ps4fancontrol.service /etc/systemd/system
sudo systemctl daemon-reload
sudo systemctl enable ps4fancontrol
sudo systemctl start ps4fancontrol
sudo systemctl status ps4fancontrol
```
