# Nest Keeper

Nest Keeper is a community management system built with a C backend and a browser-based frontend.
It manages flats, residents, payment dues, and admin-logged complaints for a housing community through a local web server.

## Overview

The project started as a terminal-based C application and was extended into a small full-stack app:

- The backend is written in C and runs an HTTP server on `localhost:8080`
- The frontend is built with HTML, CSS, Bootstrap, and JavaScript
- Data is persisted locally using `.dat` files
- Complaints are prioritized using a heap-backed queue

## Features

- Add flats in blocks `A` to `E`
- Add residents to existing empty flats
- Edit occupant details
- Delete flats when they are empty and have no pending dues or active complaints
- Add multiple dues for the same flat
- Process dues in queue order
- Log complaints on behalf of residents
- View complaints in priority order
- Resolve complaints by removing them from the active queue
- Prevent resident removal until all dues for that flat are cleared
- Responsive dashboard UI with activity feed and map embed

## Business Rules

- A resident cannot be removed if that flat has pending dues
- A flat cannot be deleted if it is occupied
- A flat cannot be deleted if it has pending dues
- A flat cannot be deleted if it has an active complaint
- Phone numbers must contain exactly `10` digits
- Multiple pending dues are allowed for the same occupied flat
- Payment processing follows queue order
- Complaints remain active until they are marked as `Resolved`

## Tech Stack

- Backend: C
- Frontend: HTML, CSS, JavaScript
- UI: Bootstrap
- Networking: Winsock
- Persistence: local binary `.dat` files
- Complaint Priority: max-heap

## Project Structure

```text
SDP/
|-- main.c
|-- headers/
|   |-- house.h
|   |-- resident.h
|   |-- payment.h
|   |-- complaint.h
|   `-- file_io.h
|-- house_modules/
|   |-- house_structure.c
|   `-- house_search.c
|-- resident_modules/
|   `-- insert_search_delete.c
|-- payment_modules/
|   `-- payment_queue.c
|-- complaint_modules/
|   `-- complaint_heap.c
|-- file_modules/
|   `-- file_io.c
|-- frontend/
|   |-- index.html
|   |-- styles.css
|   |-- app.js
|   |-- bootstrap.min.css
|   `-- bootstrap.bundle.min.js
`-- README.md
```

## How It Works

### Backend

The backend in [main.c](C:\Users\Mithin Krishna\OneDrive\Desktop\c-workspace\SDP\main.c) starts a local web server and exposes API routes for:

- `GET /api/state`
- `POST /api/flats/add`
- `POST /api/flats/delete`
- `POST /api/residents/add`
- `POST /api/residents/update`
- `POST /api/residents/delete`
- `POST /api/payments/add`
- `POST /api/payments/process`
- `POST /api/complaints/add`
- `POST /api/complaints/update`

The backend uses the existing C modules for:

- flat creation and storage
- resident AVL tree operations
- payment queue operations
- complaint heap operations
- file persistence

### Frontend

The frontend calls the backend using `fetch()`:

1. User submits a form or clicks an action button
2. JavaScript sends a request to a backend API route
3. The backend performs the operation
4. The frontend reloads `/api/state`
5. The dashboard refreshes with the latest data

## Data Files

The app stores runtime data locally:

- `flats.dat`
- `database.dat`
- `payments.dat`
- `complaints.dat`

These files are used to restore the application state on restart.

## Build Instructions

Use GCC on Windows.

```powershell
gcc main.c file_modules\file_io.c house_modules\house_structure.c house_modules\house_search.c payment_modules\payment_queue.c resident_modules\insert_search_delete.c complaint_modules\complaint_heap.c -o community_web.exe -lws2_32
```

## Run Instructions

```powershell
.\community_web.exe
```

Then open:

- [http://localhost:8080](http://localhost:8080)

## Frontend Notes

- The frontend is served directly by the C backend
- The UI is responsive for desktop and mobile
- Bootstrap assets are included in the `frontend/` folder
- A Google Maps iframe is embedded under the `Nest Keeper` heading
- The dashboard is split into operations, records, and monitoring sections

## Notes for Review

If you need to explain this project briefly in a review:

> The frontend and backend are integrated through API calls.  
> The frontend sends requests to the C backend, the backend processes the logic and updates stored data, and then the frontend fetches the latest state again to keep the UI in sync.

## Current Limitations

- The app is designed for local use on Windows
- There is no authentication layer
- Data is stored in local binary files instead of a database server
- The backend is a simple custom HTTP server, not a full framework
- Complaint history is not archived separately after resolution

## Future Improvements

- Add authentication and role-based access
- Add resident search filters by more fields
- Add payment history and receipts
- Add complaint history and reporting after resolution
- Add dashboard charts and reporting
- Add export options for admin records
