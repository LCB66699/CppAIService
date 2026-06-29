-- CppAIService 数据库初始化脚本
-- MySQL 容器首次启动时自动执行

CREATE DATABASE IF NOT EXISTS chat_service CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE chat_service;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(64) NOT NULL UNIQUE,
    password VARCHAR(256) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_username (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 聊天消息表
CREATE TABLE IF NOT EXISTS chat_message (
    id BIGINT NOT NULL COMMENT 'user_id',
    username VARCHAR(64) NOT NULL,
    session_id VARCHAR(128) NOT NULL,
    is_user TINYINT NOT NULL DEFAULT 1 COMMENT '1=用户消息, 0=AI回复',
    content TEXT NOT NULL,
    ts BIGINT NOT NULL COMMENT '消息时间戳(毫秒)',
    INDEX idx_user_session (id, session_id),
    INDEX idx_ts (ts)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
