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
        activityFeed.innerHTML = `<li><strong>No actions yet</strong><span>Backend events and frontend actions will appear here.</span></li>`;
        return;
    }

    activityFeed.innerHTML = activityEntries
        .map(
            (entry) => `
                <li>
                    <strong>${entry.title}</strong>
                    <span>${entry.detail} • ${entry.time}</span>
                </li>
            `
        )
        .join("");
}

function showToast(message, isError = false) {
    toast.textContent = message;
    toast.style.background = isError ? "rgba(180, 71, 60, 0.96)" : "rgba(30, 42, 47, 0.94)";
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
        createStatCard("Pending Dues", `₹${Number(summary.pendingAmount ?? 0).toFixed(2)}`),
    ].join("");
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
        flatsBody.innerHTML = `<tr><td colspan="4" class="empty-state">No flats match the current filters.</td></tr>`;
        return;
    }

    flatsBody.innerHTML = flats
        .map(
            (flat) => `
                <tr>
                    <td>${flat.block}</td>
                    <td>${flat.flatNo}</td>
                    <td>${flat.bhk} BHK</td>
                    <td>
                        <span class="badge ${flat.status === 1 ? "occupied" : "available"}">
                            ${flat.status === 1 ? "Occupied" : "Available"}
                        </span>
                    </td>
                </tr>
            `
        )
        .join("");
}

function renderResidents() {
    const query = document.getElementById("resident-search").value.trim().toLowerCase();
    const residents = [...state.data.residents]
        .filter((resident) => {
            if (!query) return true;
            return [resident.block, resident.flatNo, resident.name, resident.phone]
                .join(" ")
                .toLowerCase()
                .includes(query);
        })
        .sort((a, b) => `${a.block}${a.flatNo}`.localeCompare(`${b.block}${b.flatNo}`));

    if (!residents.length) {
        residentsBody.innerHTML = `<tr><td colspan="5" class="empty-state">No residents found.</td></tr>`;
        return;
    }

    residentsBody.innerHTML = residents
        .map(
            (resident) => `
                <tr>
                    <td>${resident.block}</td>
                    <td>${resident.flatNo}</td>
                    <td>${resident.name}</td>
                    <td>${resident.phone}</td>
                    <td>
                        <button class="danger" data-action="remove-resident" data-block="${resident.block}" data-flat="${resident.flatNo}">
                            Remove
                        </button>
                    </td>
                </tr>
            `
        )
        .join("");
}

function renderPayments() {
    const payments = [...state.data.payments]
        .sort((a, b) => `${a.block}${a.flatNo}`.localeCompare(`${b.block}${b.flatNo}`));

    if (!payments.length) {
        paymentsBody.innerHTML = `<tr><td colspan="4" class="empty-state">No pending payments in the queue.</td></tr>`;
        return;
    }

    paymentsBody.innerHTML = payments
        .map(
            (payment) => `
                <tr>
                    <td>${payment.block}</td>
                    <td>${payment.flatNo}</td>
                    <td>₹${Number(payment.amountDue).toFixed(2)}</td>
                    <td>
                        <button class="ghost" data-action="process-payment" data-block="${payment.block}" data-flat="${payment.flatNo}">
                            Process
                        </button>
                    </td>
                </tr>
            `
        )
        .join("");
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
        lastRefresh.textContent = new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
        renderAll();
    } catch (error) {
        serverStatus.textContent = "Disconnected";
        showToast(error.message, true);
    }
}

async function submitForm(endpoint, form, successLabel) {
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
    form.reset();
    await fetchState();
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
            addActivity("Resident action failed", error.message);
            showToast(error.message, true);
        }
    });

    document.getElementById("payment-form").addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            await submitForm("/api/payments/add", event.currentTarget, "Payment due raised");
        } catch (error) {
            addActivity("Payment action failed", error.message);
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

        const block = button.dataset.block;
        const flatNo = button.dataset.flat;
        const endpoint = button.dataset.action === "remove-resident"
            ? "/api/residents/delete"
            : "/api/payments/process";
        const label = button.dataset.action === "remove-resident"
            ? "Resident removed"
            : "Payment processed";

        try {
            const formData = new URLSearchParams({ block, flatNo });
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
