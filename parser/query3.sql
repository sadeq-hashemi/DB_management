
SELECT COUNT(*)
FROM (SELECT COUNT(*) AS "numCat" FROM Categories GROUP BY ItemID)
WHERE numCat = 4;
