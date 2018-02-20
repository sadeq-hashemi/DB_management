SELECT COUNT(*) 
FROM AuctionUsers, (SELECT DISTINCT UserID FROM Items) AS "Sellers"
WHERE Rating > 1000 AND AuctionUsers.UserID = Sellers.UserID;
