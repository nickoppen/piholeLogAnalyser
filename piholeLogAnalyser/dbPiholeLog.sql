-- phpMyAdmin SQL Dump
-- version 5.2.2
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Apr 19, 2026 at 11:59 AM
-- Server version: 10.11.11-MariaDB
-- PHP Version: 8.2.28

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `dbPiholeLog`
--

-- --------------------------------------------------------

--
-- Table structure for table `tblClient`
--

CREATE TABLE `tblClient` (
  `MAC` varchar(17) NOT NULL,
  `clientSubNetID` tinyint(4) UNSIGNED DEFAULT NULL,
  `client` varchar(255) NOT NULL,
  `IP` varchar(24) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tblCommon`
--

CREATE TABLE `tblCommon` (
  `domainID` bigint(20) NOT NULL
) ENGINE=Aria DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci PACK_KEYS=0;

-- --------------------------------------------------------

--
-- Table structure for table `tblDomain`
--

CREATE TABLE `tblDomain` (
  `ID` bigint(20) UNSIGNED NOT NULL,
  `domain` varchar(256) NOT NULL,
  `firstSearchDate` datetime NOT NULL,
  `levelOfInterest` tinyint(4) DEFAULT 0,
  `typeID` int(11) DEFAULT 0
) ENGINE=Aria DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci PACK_KEYS=0;

-- --------------------------------------------------------

--
-- Table structure for table `tblDomainType`
--

CREATE TABLE `tblDomainType` (
  `type` varchar(255) NOT NULL,
  `loiId` int(11) NOT NULL DEFAULT 0,
  `ID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tblKnown`
--

CREATE TABLE `tblKnown` (
  `domain` varchar(255) NOT NULL,
  `levelOfInterest` tinyint(4) NOT NULL,
  `comment` varchar(255) DEFAULT NULL COMMENT 'information only',
  `type` int(11) DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tblLevelOfInterest`
--

CREATE TABLE `tblLevelOfInterest` (
  `ID` tinyint(4) NOT NULL,
  `interest` varchar(16) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tblLog`
--

CREATE TABLE `tblLog` (
  `domainID` int(11) NOT NULL,
  `clientSubNetID` tinyint(16) UNSIGNED NOT NULL,
  `status` tinyint(4) UNSIGNED NOT NULL,
  `latestSearchDate` datetime NOT NULL,
  `queryCount` int(11) NOT NULL DEFAULT 1
) ENGINE=Aria DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci PACK_KEYS=0;

-- --------------------------------------------------------

--
-- Table structure for table `tblReadLog`
--

CREATE TABLE `tblReadLog` (
  `readDateTime` datetime NOT NULL DEFAULT current_timestamp(),
  `fileDateTime` datetime NOT NULL,
  `fileSize` int(11) NOT NULL,
  `logEntriesProcessed` int(11) NOT NULL DEFAULT 0,
  `comment` varchar(256) DEFAULT NULL
) ENGINE=Aria DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci PACK_KEYS=0;

-- --------------------------------------------------------

--
-- Table structure for table `tblStatus`
--

CREATE TABLE `tblStatus` (
  `ID` tinyint(4) NOT NULL,
  `statusTxt` varchar(16) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `tblClient`
--
ALTER TABLE `tblClient`
  ADD UNIQUE KEY `MAC` (`MAC`);

--
-- Indexes for table `tblCommon`
--
ALTER TABLE `tblCommon`
  ADD PRIMARY KEY (`domainID`);

--
-- Indexes for table `tblDomain`
--
ALTER TABLE `tblDomain`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `domain` (`domain`);

--
-- Indexes for table `tblDomainType`
--
ALTER TABLE `tblDomainType`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `tblKnown`
--
ALTER TABLE `tblKnown`
  ADD PRIMARY KEY (`domain`);

--
-- Indexes for table `tblLevelOfInterest`
--
ALTER TABLE `tblLevelOfInterest`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `tblLog`
--
ALTER TABLE `tblLog`
  ADD PRIMARY KEY (`domainID`,`clientSubNetID`,`status`);

--
-- Indexes for table `tblReadLog`
--
ALTER TABLE `tblReadLog`
  ADD PRIMARY KEY (`fileDateTime`,`fileSize`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `tblDomain`
--
ALTER TABLE `tblDomain`
  MODIFY `ID` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
