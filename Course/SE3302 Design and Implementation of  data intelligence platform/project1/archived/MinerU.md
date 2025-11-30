### Miner U Using GPU

If your device supports CUDA and meets the GPU requirements of the mainline environment, you can use GPU acceleration. Please select the appropriate guide based on your system:

- [Ubuntu 22.04 LTS + GPU](docs/README_Ubuntu_CUDA_Acceleration_en_US.md)
- [Windows 10/11 + GPU](docs/README_Windows_CUDA_Acceleration_en_US.md)
- Quick Deployment with Docker
  > [!IMPORTANT]
  > Docker requires a GPU with at least 8GB of VRAM, and all acceleration features are enabled by default.
  >
  > Before running this Docker, you can use the following command to check if your device supports CUDA acceleration on Docker.
  >
  > ```bash
  > docker run --rm --gpus=all nvidia/cuda:12.1.0-base-ubuntu22.04 nvidia-smi
  > ```
  ```bash
  wget https://github.com/opendatalab/MinerU/raw/master/docker/global/Dockerfile -O Dockerfile
  docker build -t mineru:latest .
  docker run -it --name mineru --gpus=all mineru:latest /bin/bash -c "echo 'source /opt/mineru_venv/bin/activate' >> ~/.bashrc && exec bash"
  magic-pdf --help
  ```
