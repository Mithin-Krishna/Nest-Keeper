const state = {
    data: {
        summary: {},
        flats: [],
        residents: [],
        payments: [],
    },
};

const statsGrid = document.getElementById("stats-grid");
const flatsBody = document.getElementById("flats-body");
const residentsBody = document.getElementById("residents-body");
const paymentsBody = document.getElementById("payments-body");
const activityFeed = document.getElementById("activity-feed");
const toast = document.getElementById("toast");
const serverStatus = document.getElementById("server-status");
const lastRefresh = document.getElementById("last-refresh");
const editResidentForm = document.getElementById("edit-resident-form");
const editResidentModalElement = document.getElementById("editResidentModal");
const editResidentModal = window.bootstrap ? new bootstrap.Modal(editResidentModalElement) : null;

const activityEntries = [];

function addActivity(title, detail) {
    activityEntries.unshift({
        title,
        detail,
        time: new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" }),
    });

    if (activityEntries.length > 8) {
        activityEntries.pop();
    }

    renderActivity();
}

function renderActivity() {
    if (!activityEntries.length) {
        activityFeed.innerHTML = "<li><strong>No actions yet</strong><span>Frontend requests and backend responses will appear here.</span></li>";
        return;
    }

    activityFeed.innerHTML = activityEntries.map((entry) => `
        <li>
            <strong>${entry.title}</strong>
            <span>${entry.detail} | ${entry.time}</span>
        </li>
    `).join("");
}

function showToast(message, isError = false) {
    toast.textContent = message;
    toast.style.background = isError ? "rgba(186, 75, 67, 0.96)" : "rgba(31, 40, 48, 0.95)";
    toast.classList.add("show");
    window.clearTimeout(showToast.timer);
    showToast.timer = window.setTimeout(() => toast.classList.remove("show"), 2600);
}

function createStatCard(label, value) {
    return `
        <article class="stat-card">
            <p>${label}</p>
            <strong>${value}</strong>
        </article>
    `;
}

function renderStats() {
    const { summary } = state.data;
    statsGrid.innerHTML = [
        createStatCard("Total Flats", summary.flats ?? 0),
        createStatCard("Occupied Flats", summary.occupied ?? 0),
        createStatCard("Available Flats", summary.available ?? 0),
        createStatCard("Residents", summary.residents ?? 0),
        createStatCard("Pending Queue", summary.pendingPayments ?? 0),
    ].join("");
}

function hasPendingDues(block, flatNo) {
    return state.data.payments.some((payment) => payment.block === block && payment.flatNo === flatNo);
}

function isOccupied(block, flatNo) {
    return state.data.residents.some((resident) => resident.block === block && resident.flatNo === flatNo);
}

function flatMatchesFilters(flat) {
    const block = document.getElementById("flat-filter-block").value;
    const bhk = document.getElementById("flat-filter-bhk").value;
    const status = document.getElementById("flat-filter-status").value;

    return (!block || flat.block === block) &&
        (!bhk || String(flat.bhk) === bhk) &&
        (!status || String(flat.status) === status);
}

function renderFlats() {
    const flats = [...state.data.flats]
        .filter(flatMatchesFilters)
        .sort((a, b) => `${a.block}${a.flatNo}`.localeCompare(`${b.block}${b.flatNo}`));

    if (!flats.length) {
        flatsBody.innerHTML = '<tr><td colspan="5" class="empty-state">No flats match the current filters.</td></tr>';
        return;
    }

    flatsBody.innerHTML = flats.map((flat) => `
        <tr>
            <td>${flat.block}</td>
            <td>${flat.flatNo}</td>
            <td>${flat.bhk} BHK</td>
            <td>
                <span class="badge-soft ${flat.status === 1 ? "badge-occupied" : "badge-available"}">
                    ${flat.status === 1 ? "Occupied" : "Available"}
                </span>
            </td>
            <td>
                <div class="d-flex gap-2 flex-wrap">
                <button
                    class="btn btn-sm ${isOccupied(flat.block, flat.flatNo) || hasPendingDues(flat.block, flat.flatNo) ? "btn-danger-soft" : "btn-soft-secondary"}"
                    data-action="delete-flat"
                    data-block="${flat.block}"
                    data-flat="${flat.flatNo}"
                    ${(isOccupied(flat.block, flat.flatNo) || hasPendingDues(flat.block, flat.flatNo)) ? "disabled" : ""}
                    title="${isOccupied(flat.block, flat.flatNo) ? "Remove the occupant before deleting the flat." : hasPendingDues(flat.block, flat.flatNo) ? "Clear dues before deleting the flat." : "Delete flat"}">
                    Delete
                </button>
                </div>
            </td>
        </tr>
    `).join("");
}

function renderResidents() {
    const query = document.getElementById("resident-search").value.trim().toLowerCase();
    const residents = [...state.data.residents]
        .filter((resident) => {
            if (!query) {
                return true;
            }

            return [resident.block, resident.flatNo, resident.name, resident.phone]
                .join(" ")
                .toLowerCase()
                .includes(query);
        })
        .sort((a, b) => `${a.block}${a.flatNo}`.localeCompare(`${b.block}${b.flatNo}`));

    if (!residents.length) {
        residentsBody.innerHTML = '<tr><td colspan="5" class="empty-state">No residents found.</td></tr>';
        return;
    }

    residentsBody.innerHTML = residents.map((resident) => {
        const blocked = hasPendingDues(resident.block, resident.flatNo);

        return `
            <tr>
                <td>${resident.block}</td>
                <td>${resident.flatNo}</td>
                <td>${resident.name}</td>
                <td>${resident.phone}</td>
                <td>
                    <div class="d-flex gap-2 flex-wrap">
                    <button
                        class="btn btn-sm btn-soft"
                        data-action="edit-resident"
                        data-block="${resident.block}"
                        data-flat="${resident.flatNo}"
                        data-name="${resident.name}"
                        data-phone="${resident.phone}">
                        Edit
                    </button>
                    <button
                        class="btn btn-sm ${blocked ? "btn-danger-soft" : "btn-soft"}"
                        data-action="remove-resident"
                        data-block="${resident.block}"
                        data-flat="${resident.flatNo}"
                        ${blocked ? "disabled" : ""}
                        title="${blocked ? "Clear pending dues before removal." : "Remove resident"}">
                        ${blocked ? "Dues Pending" : "Remove"}
                    </button>
                    </div>
                </td>
            </tr>
        `;
    }).join("");
}

function renderPayments() {
    const payments = state.data.payments;

    if (!payments.length) {
        paymentsBody.innerHTML = '<tr><td colspan="5" class="empty-state">No pending payments in the queue.</td></tr>';
        return;
    }

    paymentsBody.innerHTML = payments.map((payment, index) => `
        <tr>
            <td><span class="badge-soft badge-warning">${index + 1}</span></td>
            <td>${payment.block}</td>
            <td>${payment.flatNo}</td>
            <td>Rs. ${Number(payment.amountDue).toFixed(2)}</td>
            <td>
                <button
                    class="btn btn-sm btn-warning-soft"
                    data-action="process-payment"
                    data-queue-index="${payment.queueIndex}">
                    Process
                </button>
            </td>
        </tr>
    `).join("");
}

function renderAll() {
    renderStats();
    renderFlats();
    renderResidents();
    renderPayments();
}

async function fetchState() {
    try {
        const response = await fetch("/api/state");
        if (!response.ok) {
            throw new Error("Backend did not return the latest state.");
        }

        state.data = await response.json();
        serverStatus.textContent = "Connected";
        lastRefresh.textContent = new Date().toLocaleTimeString([], {
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit",
        });
        renderAll();
    } catch (error) {
        serverStatus.textContent = "Disconnected";
        showToast(error.message, true);
    }
}

async function submitForm(endpoint, form, successLabel, reset = true) {
    const formData = new URLSearchParams(new FormData(form));
    const response = await fetch(endpoint, {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: formData.toString(),
    });

    const payload = await response.json();
    if (!response.ok || !payload.success) {
        throw new Error(payload.message || "The request could not be completed.");
    }

    addActivity(successLabel, payload.message);
    showToast(payload.message);
    if (reset) {
        form.reset();
    }
    await fetchState();
    return payload;
}

function fillEditResidentForm(block, flatNo, name, phone) {
    editResidentForm.elements.block.value = block;
    editResidentForm.elements.flatNo.value = flatNo;
    editResidentForm.elements.name.value = name;
    editResidentForm.elements.phone.value = phone;
}

function wireForms() {
    document.getElementById("flat-form").addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            await submitForm("/api/flats/add", event.currentTarget, "Flat created");
        } catch (error) {
            addActivity("Flat creation failed", error.message);
            showToast(error.message, true);
        }
    });

    document.getElementById("resident-form").addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            await submitForm("/api/residents/add", event.currentTarget, "Resident added");
        } catch (error) {
            addActivity("Resident add failed", error.message);
            showToast(error.message, true);
        }
    });

    document.getElementById("payment-form").addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            await submitForm("/api/payments/add", event.currentTarget, "Due added");
        } catch (error) {
            addActivity("Due creation failed", error.message);
            showToast(error.message, true);
        }
    });

    editResidentForm.addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            await submitForm("/api/residents/update", event.currentTarget, "Occupant updated", false);
            if (editResidentModal) {
                editResidentModal.hide();
            }
        } catch (error) {
            addActivity("Occupant update failed", error.message);
            showToast(error.message, true);
        }
    });
}

function wireFilters() {
    ["flat-filter-block", "flat-filter-bhk", "flat-filter-status"].forEach((id) => {
        document.getElementById(id).addEventListener("change", renderFlats);
    });

    document.getElementById("resident-search").addEventListener("input", renderResidents);
}

function wireTableActions() {
    document.body.addEventListener("click", async (event) => {
        const button = event.target.closest("button[data-action]");
        if (!button) {
            return;
        }

        const action = button.dataset.action;

        if (action === "edit-resident") {
            fillEditResidentForm(
                button.dataset.block,
                button.dataset.flat,
                button.dataset.name,
                button.dataset.phone
            );
            if (editResidentModal) {
                editResidentModal.show();
            }
            return;
        }

        try {
            let endpoint = "";
            let formData;
            let label = "";

            if (action === "remove-resident") {
                endpoint = "/api/residents/delete";
                formData = new URLSearchParams({
                    block: button.dataset.block,
                    flatNo: button.dataset.flat,
                });
                label = "Resident removed";
            } else if (action === "delete-flat") {
                endpoint = "/api/flats/delete";
                formData = new URLSearchParams({
                    block: button.dataset.block,
                    flatNo: button.dataset.flat,
                });
                label = "Flat deleted";
            } else if (action === "process-payment") {
                endpoint = "/api/payments/process";
                formData = new URLSearchParams({
                    queueIndex: button.dataset.queueIndex,
                });
                label = "Payment processed";
            } else {
                return;
            }

            const response = await fetch(endpoint, {
                method: "POST",
                headers: { "Content-Type": "application/x-www-form-urlencoded" },
                body: formData.toString(),
            });
            const payload = await response.json();

            if (!response.ok || !payload.success) {
                throw new Error(payload.message || "Action failed.");
            }

            addActivity(label, payload.message);
            showToast(payload.message);
            await fetchState();
        } catch (error) {
            addActivity("Action failed", error.message);
            showToast(error.message, true);
        }
    });
}

wireForms();
wireFilters();
wireTableActions();
renderActivity();
fetchState();