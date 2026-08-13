-- Таблица заказов с поддержкой сессий
CREATE TABLE IF NOT EXISTS orders (
    order_id BIGSERIAL PRIMARY KEY,
    product_id INTEGER NOT NULL,
    amount INTEGER NOT NULL CHECK (amount > 0),
    email_client VARCHAR(255) NOT NULL,
    price DECIMAL(10, 2) NOT NULL CHECK (price >= 0),
    phone_number VARCHAR(20) NOT NULL,
    session_id VARCHAR(36)
);

-- Индексы для быстрого поиска
CREATE INDEX idx_orders_session_id ON orders(session_id);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_email_client ON orders(email_client);
CREATE INDEX idx_orders_created_at ON orders(created_at);

-- Комментарии к таблице и колонкам
COMMENT ON TABLE orders IS 'Таблица заказов';
COMMENT ON COLUMN orders.order_id IS 'Уникальный идентификатор заказа';
COMMENT ON COLUMN orders.product_id IS 'ID товара';
COMMENT ON COLUMN orders.amount IS 'Количество товара';
COMMENT ON COLUMN orders.email_client IS 'Email клиента';
COMMENT ON COLUMN orders.price IS 'Цена за единицу товара';
COMMENT ON COLUMN orders.phone_number IS 'Номер телефона клиента';
COMMENT ON COLUMN orders.session_id IS 'ID сессии (UUID)';