# Используем кэширование для ускорения сборки
FROM ubuntu:22.04 AS vcpkg-base

# Установка vcpkg и библиотек (отдельный слой для кэширования)
RUN apt-get update && apt-get install -y \
    build-essential \
    bison \
    flex \
    cmake \
    git \
    curl \
    tar \
	librdkafka-dev \
    zip \
    unzip \
    autoconf \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Клонирование vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

# Установка библиотек (этот слой будет кэшироваться отдельно)
# Разбиваем установку на отдельные шаги для лучшего кэширования
RUN vcpkg install \
    boost-asio:x64-linux \
    boost-beast:x64-linux \
    boost-system:x64-linux \
    boost-uuid:x64-linux \
    nlohmann-json:x64-linux


# Установка libpqxx через apt
RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libpqxx-6.4 \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

# Проверяем установленные пакеты
RUN echo "=== Installed vcpkg packages ===" && \
    vcpkg list

# Builder этап
FROM vcpkg-base AS builder

WORKDIR /app
COPY . .

# Диагностика
RUN echo "=== Проверка переменных ===" && \
    echo "VCPKG_ROOT=${VCPKG_ROOT}" && \
    echo "=== Проверка файлов проекта ===" && \
    ls -la /app/ && \
    echo "=== Проверка CMakeLists.txt ===" && \
    cat /app/CMakeLists.txt

# Сборка (исправлено!)
RUN mkdir -p build && cd build && \
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DVCPKG_TARGET_TRIPLET=x64-linux && \
    make -j$(nproc)

# Runtime этап
FROM ubuntu:22.04

# Устанавливаем runtime зависимости
RUN apt-get update && apt-get install -y \
    libssl3 \
    zlib1g \
    ca-certificates \
    librdkafka-dev \
    libpqxx-6.4 \
    && rm -rf /var/lib/apt/lists/*

# Копируем библиотеки
COPY --from=vcpkg-base /opt/vcpkg/installed/x64-linux/lib/*.so* /usr/local/lib/
RUN ldconfig

WORKDIR /app

# ✅ Копируем бинарник
COPY --from=builder /app/build/OrderService /app/OrderService

# ✅ Копируем исходники (для отладки в VS Code)
COPY --from=builder /app/src /app/src
COPY --from=builder /app/CMakeLists.txt /app/CMakeLists.txt

EXPOSE 8181

CMD ["/app/OrderService"]