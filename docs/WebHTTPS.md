# Web 远程桌面 HTTPS 配置指南（新手完整版）

本指南帮助你从零配置 HTTPS，实现手机/平板通过浏览器访问远程桌面。

## 方案架构图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              互联网                                          │
│                                                                             │
│    ┌──────────┐          ┌─────────────────────────────────────────────┐   │
│    │  手机    │          │           公网服务器 (Linux/Windows)         │   │
│    │  平板    │  HTTPS   │  ┌─────────────┐      ┌─────────────────┐   │   │
│    │  电脑    │ ──────── │  │   Nginx     │ ──── │   FRP Server    │   │   │
│    │ (浏览器) │  :8080   │  │  (SSL证书)  │ :9000│   (frps)        │   │   │
│    └──────────┘          │  └─────────────┘      └────────┬────────┘   │   │
│                          │        域名: your.domain.com   │            │   │
│                          └────────────────────────────────┼────────────┘   │
│                                                           │                │
│                                                      TCP 隧道              │
│                                                           │                │
└───────────────────────────────────────────────────────────┼────────────────┘
                                                            │
┌───────────────────────────────────────────────────────────┼────────────────┐
│                            本地网络                        │                │
│                                                           │                │
│    ┌─────────────────────────────────────┐                │                │
│    │        Windows 电脑 (主控)           │                │                │
│    │  ┌─────────────┐   ┌─────────────┐  │   ┌────────────▼───────────┐   │
│    │  │    Yama     │───│ FRP Client  │◄─┼───│      FRP 隧道          │   │
│    │  │  Web服务    │   │   (frpc)    │  │   └────────────────────────┘   │
│    │  │   :9000     │   └─────────────┘  │                                │
│    │  └──────┬──────┘                    │                                │
│    └─────────┼───────────────────────────┘                                │
│              │                                                            │
│              │ 远程控制                                                    │
│              ▼                                                            │
│    ┌──────────────────────────────────────────────────────────────────┐   │
│    │                 被控设备 (支持 Windows / Linux)                    │   │
│    │                                                                  │   │
│    │    ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐      │   │
│    │    │ Windows A │ │ Windows B │ │  Linux A  │ │  Linux B  │ ...  │   │
│    │    └───────────┘ └───────────┘ └───────────┘ └───────────┘      │   │
│    └──────────────────────────────────────────────────────────────────┘   │
│                                                                           │
└───────────────────────────────────────────────────────────────────────────┘
```

**数据流向**：手机浏览器 → HTTPS → Nginx → FRP Server → FRP Client → Yama → 被控设备

---

## Web 远程桌面的优势

### 随时随地，即开即用

- **无需安装客户端** - 打开浏览器就能用，手机、平板、任意电脑都行
- **跨平台支持** - iOS、Android、Windows、Mac、Linux 通吃
- **移动办公** - 出差、外出时用手机就能处理紧急事务

### 多人协作，团队共享

- **多人同时在线** - 传统 Windows 客户端一人使用时他人无法登录，Web 端支持多人同时访问
- **权限分离** - 不同成员可同时监控不同设备，互不干扰
- **便于展示** - 分享链接即可让同事/客户查看，无需安装任何软件

### 降低成本，提升效率

- **零部署成本** - 无需在每台电脑上安装客户端软件
- **快速响应** - 收到告警通知，掏出手机就能处理
- **安全加密** - HTTPS 传输，数据安全有保障

---

## 前置条件

在开始配置前，请确认你已具备以下条件：

| 条件 | 说明 | 必需 |
|------|------|------|
| **Yama 程序授权** | 需要有效的软件授权才能使用 Web 功能 | ✅ |
| **Windows 电脑** | 用于运行 Yama 主控程序（见下方说明） | ✅ |
| **公网服务器** | 用于运行 FRP + Nginx，需有固定公网 IP | ✅ |
| **域名** | 用于申请 HTTPS 证书，可购买或免费获取 | ✅ |

> 💡 **关于 Windows 电脑**：Yama 主控程序需要在 Windows 上运行。如果你的公网服务器是 Windows 系统，可以直接在上面运行 Yama；如果公网服务器是 Linux，则需要另外准备一台 Windows 电脑。

### 没有这些条件？

如果你有 Web 远程桌面的需求，但缺少上述条件，欢迎联系作者：

- **咨询方案** - 我们可以提供服务器、域名等一站式解决方案
- **购买授权** - 获取 Yama 程序授权，解锁全部功能
- **技术支持** - 协助完成配置，快速上手使用

📬 **Telegram**：[@doge_grandfather](https://t.me/doge_grandfather)

---

## 目录

1. [为什么需要 HTTPS](#1-为什么需要-https)
2. [技术准备](#2-技术准备)
3. [获取域名](#3-获取域名)
4. [配置域名解析](#4-配置域名解析)
5. [申请 SSL 证书](#5-申请-ssl-证书)
6. [安装配置 Nginx](#6-安装配置-nginx)
7. [配置 FRP](#7-配置-frp)
8. [启动服务](#8-启动服务)
9. [手机访问测试](#9-手机访问测试)
10. [常见问题](#10-常见问题)

---

## 1. 为什么需要 HTTPS

浏览器出于安全考虑，H264 视频解码功能只能在**安全上下文**中使用：

| 访问方式 | H264 远程桌面 |
|----------|---------------|
| `https://任意地址` | ✅ 支持 |
| `http://localhost` 或 `http://127.0.0.1` | ✅ 支持 |
| `http://其他地址`（包括内网 IP、公网 IP、域名） | ❌ 不支持 |

**简单说：**
- **没有 HTTPS** → 只能在本机浏览器访问（localhost）
- **配置 HTTPS** → 手机、平板、其他电脑都能访问

如果用 HTTP 从非本机访问，会提示：
```
Browser does not support H264: VideoDecoder not available
```

本指南帮你配置 HTTPS，实现随时随地访问。

---

## 2. 技术准备

确认你已完成以下准备：

| 项目 | 说明 |
|------|------|
| FRP 基础配置 | 已配置好 FRP 内网穿透（frps 在服务器运行，frpc 在本机运行） |
| SSH 工具 | 如 PuTTY、Xshell、Windows Terminal，用于连接服务器执行命令 |
| Yama Web 服务 | 已在 Yama 中启用 Web 服务（菜单 → 设置 → Web 服务） |

> 💡 如果还没配置 FRP，请先参考 FRP 官方文档完成基础配置。

**最终架构：**

```
手机浏览器 ──HTTPS──> 你的域名:端口 ──> Nginx ──> FRP ──> 你的电脑
```

---

## 3. 获取域名

### 方案一：购买域名（推荐）

便宜的域名一年只要几块钱：

| 注册商 | 网址 | 说明 |
|--------|------|------|
| 阿里云 | https://wanwang.aliyun.com | 国内，需实名 |
| 腾讯云 | https://dnspod.cloud.tencent.com | 国内，需实名 |
| Cloudflare | https://www.cloudflare.com | 国外，无需实名 |
| Namesilo | https://www.namesilo.com | 国外，便宜 |

> 💡 **提示**：`.icu`、`.top`、`.xyz` 等后缀很便宜，首年通常 ¥5-10。

### 方案二：免费域名

| 服务 | 网址 | 说明 |
|------|------|------|
| DuckDNS | https://www.duckdns.org | 免费子域名，如 `xxx.duckdns.org` |
| No-IP | https://www.noip.com | 免费子域名 |
| Freenom | https://www.freenom.com | 免费顶级域名（不稳定） |

---

## 4. 配置域名解析

将域名指向你的**公网服务器 IP**。

### 4.1 登录域名管理面板

去你购买域名的网站，找到「DNS 解析」或「域名解析」。

### 4.2 添加 A 记录

| 字段 | 填写 |
|------|------|
| 记录类型 | A |
| 主机记录 | `@`（代表根域名）或 `www` |
| 记录值 | 你的服务器公网 IP，如 `123.45.67.89` |
| TTL | 600（或默认） |

### 4.3 验证解析生效

等待 1-5 分钟，然后测试：

```bash
ping your.domain.com
```

如果返回你的服务器 IP，说明解析成功。

---

## 5. 申请 SSL 证书

使用 Let's Encrypt 免费证书。

### 5.1 SSH 连接到服务器

使用 SSH 工具连接到你的公网服务器。

### 5.2 安装 Certbot

```bash
# Ubuntu / Debian
apt update && apt install certbot -y

# CentOS
yum install certbot -y
```

### 5.3 申请证书

使用 DNS 验证方式，**不需要占用 80/443 端口**：

```bash
certbot certonly --manual --preferred-challenges dns -d your.domain.com
```

> ⚠️ 将 `your.domain.com` 替换为你的实际域名

### 5.4 按提示操作

1. **输入邮箱**：用于接收证书到期提醒
2. **同意条款**：输入 `Y`
3. **分享邮箱**：输入 `N`（可选）

### 5.5 添加 DNS TXT 记录

Certbot 会显示类似信息：

```
Please deploy a DNS TXT record under the name:
_acme-challenge.your.domain.com
with the following value:
aBcDeFgHiJkLmNoPqRsTuVwXyZ1234567890
```

**去域名管理面板添加 TXT 记录：**

| 字段 | 填写 |
|------|------|
| 记录类型 | TXT |
| 主机记录 | `_acme-challenge` |
| 记录值 | Certbot 显示的那串字符 |
| TTL | 600 |

### 5.6 验证 TXT 记录生效

```bash
nslookup -type=TXT _acme-challenge.your.domain.com
```

看到记录值后，**回到 Certbot 按回车**继续。

### 5.7 证书申请成功

成功后显示：

```
Successfully received certificate.
Certificate is saved at: /etc/letsencrypt/live/your.domain.com/fullchain.pem
Key is saved at:         /etc/letsencrypt/live/your.domain.com/privkey.pem
```

**记住这两个路径，后面要用。**

### 5.8 设置自动续期（可选）

证书有效期 90 天，建议设置自动续期：

```bash
# 方法一：直接添加定时任务（推荐）
(crontab -l 2>/dev/null; echo "0 0 1 * * certbot renew --quiet && systemctl reload nginx") | crontab -

# 方法二：手动编辑
crontab -e
# 添加这行：0 0 1 * * certbot renew --quiet && systemctl reload nginx
# 按 Ctrl+O 保存，Ctrl+X 退出
```

> 💡 这会在每月 1 号自动检查并续期证书。

---

## 6. 安装配置 Nginx

### 6.1 安装 Nginx

```bash
# Ubuntu / Debian
apt update && apt install nginx -y

# CentOS
yum install nginx -y && systemctl enable nginx
```

### 6.2 删除默认配置（避免端口冲突）

```bash
# Ubuntu / Debian
rm -f /etc/nginx/sites-enabled/default

# CentOS - 跳过此步（我们用 8080 端口，不冲突）
```

### 6.3 创建配置文件

> ⚠️ **重要**：把命令中的 `your.domain.com` 替换为你的实际域名（共 4 处）

**Ubuntu / Debian：**

```bash
cat > /etc/nginx/sites-available/yama << 'EOF'
server {
    listen 8080 ssl;
    server_name your.domain.com;

    ssl_certificate /etc/letsencrypt/live/your.domain.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/your.domain.com/privkey.pem;

    location / {
        proxy_pass http://127.0.0.1:9000;
        proxy_http_version 1.1;

        # WebSocket 支持（重要）
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;

        # 长连接超时
        proxy_read_timeout 86400;
    }
}
EOF
```

**CentOS：**

```bash
cat > /etc/nginx/conf.d/yama.conf << 'EOF'
server {
    listen 8080 ssl;
    server_name your.domain.com;

    ssl_certificate /etc/letsencrypt/live/your.domain.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/your.domain.com/privkey.pem;

    location / {
        proxy_pass http://127.0.0.1:9000;
        proxy_http_version 1.1;

        # WebSocket 支持（重要）
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;

        # 长连接超时
        proxy_read_timeout 86400;
    }
}
EOF
```

> 💡 **端口说明**：这里用 `8080`，你可以改成其他未被占用的端口（如 9443）。

### 6.4 启用配置（仅 Ubuntu/Debian 需要）

```bash
ln -s /etc/nginx/sites-available/yama /etc/nginx/sites-enabled/
```

> CentOS 已在上一步直接创建到 `conf.d` 目录，无需此步骤。

### 6.5 测试并启动

```bash
nginx -t && systemctl restart nginx
```

看到以下输出说明配置正确：
```
nginx: the configuration file /etc/nginx/nginx.conf syntax is ok
nginx: configuration file /etc/nginx/nginx.conf test is successful
```

如果报错，检查域名是否替换正确、证书路径是否存在。

---

## 7. 配置 FRP

### 7.1 FRP 服务端配置 (frps.toml)

在公网服务器上：

```toml
bindPort = 7000
```

### 7.2 FRP 客户端配置

#### 方式一：通过 Yama 程序配置（推荐）

1. 打开 Yama 主控程序
2. 菜单 → 设置 → FRP 设置
3. 启用 FRP
4. 设置 Web 端口为 `9000`
5. 保存

无需手动编辑配置文件。

#### 方式二：手动编辑 frpc.toml

在你的本机（Windows）：

```toml
serverAddr = "你的服务器IP或域名"
serverPort = 7000

[[proxies]]
name = "yama-web"
type = "tcp"
localPort = 9000
remotePort = 9000
```

> 💡 **说明**：FRP 把服务器的 9000 端口转发到你本机的 9000 端口。

---

## 8. 启动服务

### 8.1 开放防火墙端口（重要！）

很多新手卡在这一步。**服务器防火墙必须开放以下端口**：

```bash
# 查看防火墙状态
ufw status          # Ubuntu
firewall-cmd --state # CentOS

# Ubuntu - 开放端口
ufw allow 8080      # Nginx HTTPS 端口
ufw allow 7000      # FRP 端口
ufw allow 9000      # FRP Web 转发端口

# CentOS - 开放端口
firewall-cmd --permanent --add-port=8080/tcp
firewall-cmd --permanent --add-port=7000/tcp
firewall-cmd --permanent --add-port=9000/tcp
firewall-cmd --reload
```

> ⚠️ **云服务器还需要在控制台安全组中开放这些端口！**（阿里云、腾讯云等）

### 8.2 启动顺序

按以下顺序启动：

1. **服务器上**：启动 FRP Server
   ```bash
   ./frps -c frps.toml
   ```

2. **服务器上**：确认 Nginx 在运行
   ```bash
   systemctl status nginx
   ```

3. **本机上**：启动 Yama 主控程序

> 💡 **提示**：如果在 Yama 中启用了 FRP 功能，程序启动时会自动连接 FRP Server，无需手动启动 frpc。

### 8.3 流量路径

```
手机浏览器
    │
    │ HTTPS (端口 8080)
    ▼
  Nginx (SSL 终结)
    │
    │ HTTP (端口 9000)
    ▼
  FRP Server
    │
    │ TCP 隧道
    ▼
  FRP Client
    │
    │ 本地 (端口 9000)
    ▼
  Yama WebService
```

---

## 9. 手机访问测试

### 9.1 打开浏览器

在手机浏览器输入：

```
https://your.domain.com:8080
```

> ⚠️ 注意是 `https://`，不是 `http://`

### 9.2 登录

输入用户名密码登录。

### 9.3 连接远程桌面

选择设备，点击连接，享受远程桌面。

---

## 10. 常见问题

### Q: 浏览器提示"不安全"或证书错误

A: 检查域名是否和证书匹配，证书是否过期：
```bash
certbot certificates
```

### Q: 无法访问，连接超时

A: 检查各服务是否运行：
```bash
systemctl status nginx
netstat -tlnp | grep 8080
netstat -tlnp | grep 9000
```

### Q: WebSocket 连接失败

A: 确认 Nginx 配置中有 `Upgrade` 和 `Connection` 头配置。

### Q: 仍然提示 VideoDecoder not available

A:
1. 确认使用 `https://` 访问
2. 清除浏览器缓存后重试
3. 换个浏览器试试（推荐 Chrome/Edge）

### Q: 证书过期了

A: 手动续期：
```bash
certbot renew
systemctl reload nginx
```

### Q: FRP 连接不上

A: 参考 [8.1 开放防火墙端口](#81-开放防火墙端口重要) 检查防火墙配置。

### Q: 云服务器（阿里云/腾讯云）无法访问

A: 除了服务器防火墙，还需要在**云控制台的安全组**中开放端口：
1. 登录云服务器控制台
2. 找到「安全组」设置
3. 添加入站规则，开放 8080、7000、9000 端口

### Q: 如何测试各环节是否正常

A: 逐步排查：
```bash
# 1. 测试 Nginx 是否监听
curl -k https://127.0.0.1:8080

# 2. 测试 FRP 是否转发
curl http://127.0.0.1:9000

# 3. 测试域名是否解析正确
ping your.domain.com

# 4. 测试外部访问（在手机上）
# 打开 https://your.domain.com:8080
```

---

## 总结

| 步骤 | 操作位置 | 说明 |
|------|----------|------|
| 获取域名 | 域名注册商 | 购买或免费获取 |
| 域名解析 | 域名管理面板 | A 记录指向服务器 IP |
| 申请证书 | 服务器 | Let's Encrypt 免费证书 |
| 配置 Nginx | 服务器 | SSL 终结 + 反向代理 |
| 开放防火墙 | 服务器 + 云控制台 | 开放 8080、7000、9000 端口 |
| 配置 FRP | 服务器 + 本机 | 内网穿透 |
| 访问测试 | 手机 | `https://域名:端口` |

配置完成后，你就可以随时随地用手机访问远程桌面了！

---

**遇到问题？** 查看 [常见问题](#10-常见问题) 或逐步排查各环节。
