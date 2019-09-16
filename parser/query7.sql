SELECT COUNT(DISTINCT c.Category)
FROM Bids, Categories c
WHERE Bids.Amount > 100 AND Bids.ItemID = c.ItemID;
