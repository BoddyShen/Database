# Buffer-Managed Database Engine

This project is based on UMass CS645, Database Design and Implementation. The goal of this project is using IMDb movie data to implement simple database engine, including page, buffer manager, B+ Tree index, query operators.
Please refer each lab's README.md to run the project.<br> The reports of each lab are also included in each folder.

[lab1](https://github.com/BoddyShen/UMass-CS645/tree/main/lab1)

[lab2](https://github.com/BoddyShen/UMass-CS645/tree/main/lab2)

[lab3](https://github.com/BoddyShen/UMass-CS645/tree/main/lab3)


### Lab1 Buffer manager and storage
#### Objective<br>
The goal of this assignment is to implement a buffer manager for a simple database management system. The buffer manager is responsible for loading pages from a flat file and managing them in memory. The system only supports only inserting and reading rows into a single table. There is no need to handle deletions or updates.<br>

The implementation must be in C++ or Java. If you plan to use any external library beyond the basic language libraries, you need to ask for permission. The examples in this page are provided for Java.<br>

#### Overview<br>
The Buffer Manager receives page requests from high-level code (the caller). The buffer manager maintains a fixed number of frames in memory. When a page is requested, the buffer manager checks if it is already in memory; if not, it loads the page from disk. If the buffer is full, a replacement policy (e.g., Least Recently Used) must be used to evict a page. The buffer manager must also track which pages are dirty (modified) and ensure they are written back to disk before being evicted.<br>

The buffer manager stores pages in a buffer pool. For fast page retrieval, use a page table to map pageIds to frames in the buffer pool and to keep track of the page metadata. The buffer manager deals with generic Page objects. The content of the pages is opaque to the buffer manager. In this class, we will use pages to store Records from a table containing movies. The DataPage class has the logic to store and read records in pages.

![Screenshot 2025-05-08 at 12 01 37 AM](https://github.com/user-attachments/assets/fe255853-fe0b-4a78-b0ce-1b14af7241ea)

### Lab2 B+ tree index
#### Objective<br>
The goal of this assignment is to implement an unclustered B+ tree index on top of the buffer manager implemented in the previous lab.<br>

#### Overview<br>
A B+ tree index has two types of index pages: non-leaf and leaf pages. Non-leaf pages contain index entries, each including a sequence of values of the search attribute and a page pointers. Leaf pages contain data entries stored using Alternative 2: each entry is in the form <k, rid>, where k is a value of the search attribute and rid is a record id. A record id is in the form <pid, sid>, where pid is a page id and sid is a slot id within the page. Your implementation will have to support insertions, point queries, and range queries.<br>
![Screenshot 2025-05-08 at 12 02 17 AM](https://github.com/user-attachments/assets/37a8291a-f7c4-44c6-97dd-037b62dfecdf)

### Lab 3 Query execution
#### Objective<br>
The goal of this assignment is to implement a simple query executor on top of the components you developed in the first two labs.<br>

#### Overview<br>
We will consider two additional tables besides the Movies table. The schema of the tables in this database is the following. <br>

Movies (movieId: char(9), title: char(30))<br>
WorkedOn (movieId: char(9), personId: char(10), category: char(20))<br>
People (personId: char(10), name: char(105))<br>
We will implement a query executor for the following query, which finds the name of the director(s) of movies having titles in a range:

SELECT title, name <br>
FROM Movies, WorkedOn, People <br>
WHERE title >= start-range AND title <= end-range AND category = “director” AND Movies.movieId = WorkedOn.movieId AND WorkedOn.personId = People.personID <br>
The implementation uses the following fixed plan, where all joins use the Block Nested Loop join algorithm (BNL).<br>

![Screenshot 2025-05-08 at 12 02 50 AM](https://github.com/user-attachments/assets/12aa81ea-1b5b-47ea-9503-3ff0169611c5)
