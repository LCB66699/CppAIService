#!/bin/bash
set -e

# 等待 MySQL 就绪
if [ -n "$MYSQL_HOST" ]; then
    echo "Waiting for MySQL ($MYSQL_HOST:$MYSQL_PORT)..."
    until mysqladmin ping -h"$MYSQL_HOST" -P"${MYSQL_PORT:-3306}" --silent 2>/dev/null; do
        sleep 2
    done
    echo "MySQL is ready."
fi

# 等待 RabbitMQ 就绪
if [ -n "$RABBITMQ_HOST" ]; then
    echo "Waiting for RabbitMQ ($RABBITMQ_HOST:${RABBITMQ_PORT:-5672})..."
    until nc -z "$RABBITMQ_HOST" "${RABBITMQ_PORT:-5672}" 2>/dev/null; do
        sleep 2
    done
    echo "RabbitMQ is ready."
fi

exec "$@"
