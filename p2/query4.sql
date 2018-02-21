SELECT ItemID
FROM Items, (SELECT DISTINCT MAX(Currently) AS "maxCurr"
             FROM Items) AS "Max"
WHERE Currently = Max.maxCurr;
