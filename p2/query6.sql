
SELECT COUNT(DISTINCT Bids.userID)
FROM Bids, Items
WHERE Bids.userID = Items.userID;
