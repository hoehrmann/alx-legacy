DROP VIEW IF EXISTS view_dbg_rfp_is_subset;
CREATE VIEW view_dbg_rfp_is_subset AS
WITH
count_rfps AS (
  SELECT
    COUNT(*) AS cnt
  FROM
    (
      SELECT DISTINCT
        *
      FROM
        view_random_full_path_signature
    )
),
both AS (
  SELECT
    *
  FROM
    view_random_full_path_signature
  INTERSECT
  SELECT
    *
  FROM
    view_all_edges_signature
),
count_both AS (
  SELECT
    COUNT(*) AS cnt
  FROM
    both
),
only_rfps AS (
  SELECT
    *
  FROM
    view_random_full_path_signature
  EXCEPT
  SELECT
    *
  FROM
    view_all_edges_signature
),
count_only AS (
  SELECT
    COUNT(*) AS cnt
  FROM
    only_rfps
)
SELECT
  count_only.cnt = 0 AS is_subset
FROM
  count_only
