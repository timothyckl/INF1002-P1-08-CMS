# Project Specification

### 1. Introduction

A database is a structured collection of data organized in a way that allows for efficient storage, retrieval, and manipulation of that data. A database normally has many data tables, and each data table includes data for a group of similar records. The table is made up of columns and rows, and usually, the first column is used as the unique “ID” to store each record (row). For example, the data table below is used to store the prices and vendor for each fruit (“ID”).

**Data Table: FruitPrice**

| ID         | Price | Origin   |
| ---------- | ----- | -------- |
| Apple      | 3.7   | USA      |
| Orange     | 3.6   | China    |
| Watermelon | 5.6   | Malaysia |

A Database Management System (DBMS) is software that provides an interface for users interacting with databases. It is a crucial component in modern information systems and plays a central role in storing, retrieving, and managing data. DBMS provides several fundamental operations for interacting with data stored in a database. These operations typically include: Insert (Add), Query, Update (Edit), and Delete.

### 2. Detailed Requirements

Your team is required to implement a **Class Management System (CMS)** programme, a simple database management system in C with the requirements described as below. The programme will be using a command-line-like interface; no graphical UI is required.

#### 2.1. File Database

A file will be used as the database to store all the data records. Only ONE data table “StudentRecords” needs to be implemented in your file database, and there are 4 columns in the table as shown in the table below. A sample database file can be found in the “Sample-CMS.txt” file.

**StudentRecords**

| ID (data type: integer) | Name (data type: string) | Programme (data type: string) | Mark (data type: float) |
| ----------------------- | ------------------------ | ----------------------------- | ----------------------- |
| 2301234                 | Joshua Chen              | Software Engineering          | 70.5                    |
| 2201234                 | Isaac Teo                | Computer Science              | 63.4                    |
| 2304567                 | John Levoy               | Digital Supply Chain          | 85.9                    |

Please name your database file in the following convention:

- `TeamName-CMS.txt`. For example: `P1_1-CMS.txt`

> Our data file can be found in `./data/P1_8-CMS.txt`.

#### 2.2. Operations

Your programme should allow the users to conduct the following operations on the data records read from the file database.

- **OPEN**: to open the database file and read in all the records
- **SHOW ALL**: to show all the current records.
- **INSERT**: to add a new record to the database.
- **QUERY**: to search for a record in the database.
- **UPDATE**: to update the value for a given record.
- **DELETE**: to delete a record from the database.
- **SAVE**: to save all the records to the database file

##### Operation Description:

- **OPEN**: Open the database file and read in all the records.
- **SHOW ALL**: Display all the current records in the read-in data.
- **INSERT**: Insert a new data record.
  - If there is already a record with the same student ID, an error message will be displayed and the insertion will be cancelled.
  - If there is no record with the same student ID, then ask the user to enter the data for each “Column” of the new record.
- **QUERY**: Search if there is any existing record with a given student ID.
  - If a record with the same student ID exists, then display the data for the record.
  - If no record with the same student ID exists, display a warning message to indicate there is no record found.
- **UPDATE**: Update the data for a record with a given student ID.
  - If no record with the same student ID exists, display a warning message to indicate there is no record found.
  - If a record with the same student ID exists, then ask for the new data for each “Column” of the record and update the record.
- **DELETE**: Delete the record with a given student ID.
  - If no existing record with the same student ID exists, display a warning message to indicate there is no record found.
  - If a record with the same student ID exists, double confirm with the user for deletion. If confirmed, then delete the record. Otherwise, no deletion will be made.
- **SAVE**: Save all the current records into the database file.

### 2.3. Enhancement Features

- **Sorting Features**: Implement sorting of student records by:
  - Student ID (ascending or descending), and
  - Mark (ascending or descending).
- **Summary Statistics**: Implement summary commands, for example:
  - **SHOW SUMMARY** → Displays:
    - Total number of students
    - Average mark
    - Highest and lowest mark (with student names)
- **Unique Feature**: A unique feature derived by your team that demonstrates a number of the features of C and fits into your CMS system naturally.

### 2.4. Test Case Design and Validation

Your team is required to come up with a comprehensive list of test cases to ensure your application works without errors. A sample format for reference:

| Test Case ID | Description                | Input Command(s)             | Expected Output                                          | Reason for Test       | Actual Result |
| ------------ | -------------------------- | ---------------------------- | -------------------------------------------------------- | --------------------- | ------------- |
| TC05         | Update Record Successfully | `UPDATE ID=250001 Mark=88.0` | `CSM: The record with ID=250001 is successfully updated` | Verifies Update Logic | Pass          |

Test cases should be practically used while creating your application but should also be listed in the report, and executed in your video presentation.

### 3. Assessment criteria

Your assignment will be assessed according to the criteria listed in the mark scheme in Table 1.

#### Table 1: Assessment Criteria

| Code (80%)                              | Report (10%)                                              | Individual Reflection (5%)                              | Video Project presentation and demonstration (5%) |
| --------------------------------------- | --------------------------------------------------------- | ------------------------------------------------------- | ------------------------------------------------- |
| Code completeness and correctness (45%) | A logic flow with all required information                | A clear and honest reflection of your own contributions | Presentation (max 5 minutes)                      |
| Code clarity (5%)                       | Code structure and data structures are clearly elaborated | Critical evaluation of the project                      | Demo time is properly managed                     |
| Code reliability (10%)                  | Well organized and easy to read                           | Well organized and easy to read                         | Demo of the developed CMS                         |

### Appendix A. Sample responses

Assume you give a name “CMS” for your programme and the user is named as “P1_1”. The database file is named “P1_1-CMS.txt”.

Below shows sample responses.

**OPEN**

```
P1_1: OPEN
CMS: The database file “P1_1-CMS.txt” is successfully opened.
```

**SHOW ALL**

```
P1_1: SHOW ALL
CMS: Here are all the records found in the table “StudentRecords”.

ID  Name  Programme  Mark
2301234 Joshua Chen Software Engineering 70.5
2201234 Isaac Teo Computer Science 63.4
2304567 John Levoy Digital Supply Chain 85.9
```

... (Other sample responses follow).

**INSERT**

```
P1_1: INSERT ID=2301234
CMS: The record with ID=2301234 already exists.
P1_1: INSERT ID=2401234 Name= Michelle Lee Programme=Information Security Mark=73.2
CMS: A new record with ID=2401234 is successfully inserted.
P1_1: SHOW ALL
CMS: Here are all the records found in the table “StudentRecords”.

ID  Name  Programme  Mark
2301234 Joshua Chen Software Engineering 70.5
2201234 Isaac Teo Computer Science 63.4
2304567 John Levoy Digital Supply Chain 85.9
2401234 Michelle Lee Information Security 73.2
```

**QUERY**

```
P1_1: QUERY ID=2501234
CMS: The record with ID=2501234 does not exist.
P1_1: QUERY ID=2401234
CMS: The record with ID=2401234 is found in the data table.
ID  Name  Programme  Mark
2401234 Michelle Lee Information Security 73.2
```

**UPDATE**

```
P1_1: UPDATE ID=2501234 Mark=69.8
CMS: The record with ID=2501234 does not exist.
P1_1: UPDATE ID=2401234 Mark=69.8
CMS: The record with ID=2401234 is successfully updated.
P1_1: UPDATE ID=2401234 Programme=Applied AI
CMS: The record with ID=2401234 is successfully updated.
P1_1: SHOW ALL
CMS: Here are all the records found in the table “StudentRecords”.

ID  Name  Programme  Mark
2301234 Joshua Chen Software Engineering 70.5
2201234 Isaac Teo Computer Science 63.4
2304567 John Levoy Digital Supply Chain 85.9
2401234 Michelle Lee Applied AI 69.8
```

**DELETE**

```
P1_1: DELETE ID=2501234
CMS: The record with ID=2501234 does not exist.
P1_1: DELETE ID=2401234
CMS: Are you sure you want to delete record with ID=2401234? Type “Y” to Confirm or type “N” to cancel.
P1_1: N
CMS: The deletion is cancelled.
P1_1: DELETE ID=2301234
CMS: Are you sure you want to delete record with ID=2301234? Type “Y” to Confirm or type “N” to cancel.
P1_1: Y
CMS: The record with ID=2301234 is successfully deleted.
P1_1: SHOW ALL
CMS: Sure. Here are all the records found in the table “StudentRecords”.

ID  Name  Programme  Mark
2201234 Isaac Teo Computer Science 63.4
2304567 John Levoy Digital Supply Chain 85.9
2401234 Michelle Lee Applied AI 69.8
```

**SAVE**

```
P1_1: SAVE
CMS: The database file “P1_1-CMS.txt” is successfully saved.
```
