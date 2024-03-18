-- DROP SCHEMA IF EXISTS hello_schema CASCADE;

-- CREATE SCHEMA IF NOT EXISTS hello_schema;

-- CREATE TABLE IF NOT EXISTS hello_schema.users (
--     name TEXT PRIMARY KEY,
--     count INTEGER DEFAULT(1)
-- );
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";

DROP SCHEMA IF EXISTS delivery_service CASCADE;

CREATE SCHEMA IF NOT EXISTS delivery_service;

CREATE TABLE IF NOT EXISTS delivery_service.courier (
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    region TEXT NOT NULL,
    transport TEXT NOT NULL,
    working_hours TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS delivery_service.order (
    id TEXT PRIMARY KEY DEFAULT uuid_generate_v4(),
    region TEXT NOT NULL,
    weight TEXT NOT NULL,
    delivery_hours TEXT NOT NULL,
    price TEXT NOT NULL,
    courier_id TEXT,
    completed_time TEXT,
    completed_date TEXT,
    foreign key(courier_id) REFERENCES delivery_service.courier(id)
);




