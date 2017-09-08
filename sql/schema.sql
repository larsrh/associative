create table pool (
  id integer not null,
  name varchar(64) not null, -- unique
  primary key (id)
);

create table file (
  id integer not null,
  uuid varchar(36) not null, -- unique
  pool_id integer not null, -- references pool (id)
  visible smallint not null,
  primary key (id, pool_id)
);

create table content_type (
  id integer not null,
  mime varchar(64) not null, -- unique
  primary key (id)
);

create table "blob" (
  id integer not null,
  file_id integer not null, -- references file (id)
  name varchar(256) not null,
  content_type_id integer not null, -- references content_type (id)
  visible smallint not null,
  primary key (id, file_id)
  -- unique (file_id, name)
);

create table prefix (
  id integer not null,
  name varchar(64) not null, -- unique
  uri varchar(256) not null,
  primary key (id)
);

create table type (
  id integer not null,
  prefix_id integer not null,
  name varchar(64) not null
  -- unique (prefix_id, name)
);

create table metadata (
  id integer not null,
  blob_id integer not null, -- references blob (id)
  predicate_prefix_id integer not null, -- references prefix (id)
  predicate varchar(256) not null,
  object_type_id integer, -- references type (id)
  object varchar(256) not null,
  visible smallint not null,
  primary key (id)
);

create table handle (
  id integer not null,
  relation integer not null,
  relation_id integer not null,
  session_id integer not null -- references session (id)
);

create table "session" (
  id integer not null,
  ready smallint not null,
  process integer not null
);

create table journal (
  id integer not null,
  session_id integer not null, -- references session (id)
  relation integer not null,
  relation_id integer not null,
  operation integer not null,
  target varchar(256),
  executed smallint not null,
  primary key (id)
);

create table ids (
  id integer not null,
  table_name varchar(32) not null, -- unique
  next_id integer not null,
  primary key (id)
);

