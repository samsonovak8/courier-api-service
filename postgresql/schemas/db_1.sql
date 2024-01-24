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
    region_id TEXT NOT NULL,
    transport TEXT NOT NULL,
    max_weight TEXT NOT NULL,
    working_time TEXT NOT NULL
);




