# 多阶段构建：编译阶段
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential g++-11 cmake make git wget ca-certificates \
    pkg-config ccache \
    libmysqlcppconn-dev \
    librabbitmq-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    libopencv-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --branch v2.0.0 https://github.com/chenshuo/muduo.git /tmp/muduo && \
    cd /tmp/muduo && \
    find . -name CMakeLists.txt -exec sed -i 's/-Werror//g' {} + && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DMUDUO_BUILD_EXAMPLES=OFF && \
    cmake --build build -j$(nproc) && \
    cmake --install build && \
    ldconfig && \
    rm -rf /tmp/muduo

RUN git clone --depth 1 https://github.com/alanxz/SimpleAmqpClient.git /tmp/simpleamqpclient && \
    cd /tmp/simpleamqpclient && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc) && \
    cmake --install . && \
    ldconfig && \
    rm -rf /tmp/simpleamqpclient

RUN wget -q https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-linux-x64-1.17.0.tgz && \
    tar xzf onnxruntime-linux-x64-1.17.0.tgz && \
    cp -r onnxruntime-linux-x64-1.17.0/include/* /usr/local/include/ && \
    cp -r onnxruntime-linux-x64-1.17.0/lib/* /usr/local/lib/ && \
    ldconfig && \
    rm -rf onnxruntime-linux-x64-1.17.0*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

COPY . /workspace
WORKDIR /workspace

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc)

# 运行阶段：最小化镜像
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    mysql-client \
    netcat-openbsd \
    libmysqlcppconn8 \
    librabbitmq4 \
    libcurl4 \
    libssl3 \
    libopencv-core4.5d \
    libopencv-imgproc4.5d \
    libopencv-highgui4.5d \
    libopencv-imgcodecs4.5d \
    libopencv-videoio4.5d \
    libopencv-dnn4.5d \
    && rm -rf /var/lib/apt/lists/*

# 从构建阶段拷贝二进制和库文件
COPY --from=builder /usr/local/lib/libmuduo_net.a /usr/local/lib/
COPY --from=builder /usr/local/lib/libmuduo_base.a /usr/local/lib/
COPY --from=builder /usr/local/lib/libSimpleAmqpClient.so* /usr/local/lib/
COPY --from=builder /usr/local/lib/libonnxruntime.so* /usr/local/lib/
COPY --from=builder /usr/local/include/onnxruntime /usr/local/include/onnxruntime

RUN ldconfig

COPY --from=builder /workspace/build/http_server /usr/local/bin/http_server
COPY --from=builder /workspace/AIApps/ChatServer/resource /opt/cppaiservice/resource
COPY docker/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 8117

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/usr/local/bin/http_server", "-p", "8117"]
