
const tg = window.Telegram.WebApp;
tg.ready();
tg.expand();

const params = new URLSearchParams(window.location.search);

const lineId = Number(params.get('line_id'));

const lineName = document.getElementById('lineName');
const courseName = document.getElementById('courseName');
const creator = document.getElementById('creator');

const positionInput = document.getElementById("position");

function getLine(lineId) {
  fetch('/api/get_lines', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      line_id: lineId
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    lineName.innerText = data.line.line_name;
    courseName.innerText = "Course: " + data.line.course_name;
    creator.innerText = "Creator: " + data.line.creator_name;
  }).catch(error => {
    console.error("Failed to retireve data: ", error);
    Telegram.WebApp.showAlert("Failed to get line data!: " + error);
  });
}

getLine(lineId);

reservationsTableBody = document.getElementById("reservationsTable").tBodies[0];

function getReservations(lineId) {
  fetch('/api/get_reservations', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      line_id: lineId
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    positionInput.max = data.user_count;

    reservationsTableBody.innerHTML = '';
    data.reservations.forEach(reserv => {
      let row = reservationsTableBody.insertRow();
      row.insertCell().textContent = reserv.position;
      row.insertCell().textContent = reserv.reserved_by;
      let newButton = document.createElement('BUTTON');
      newButton.innerText = "+";
      newButton.onclick = function() {};
      row.insertCell().appendChild(newButton);
    });
  }).catch(error => {
    console.error("Failed to retireve data: ", error);
    Telegram.WebApp.showAlert("Failed to get line data!: " + error);
  });
}

getReservations(lineId);

const reserveDialog = document.getElementById("reserveDialog");

function openReserveDialog() {
  reserveDialog.showModal();
}

function createReservation() {
  if (positionInput.valueAsNumber < Number(positionInput.min)) {
    Telegram.WebApp.showAlert("Cannot go under " + positionInput.min + "!");
    return;
  }
  if (positionInput.valueAsNumber > Number(positionInput.max)) {
    Telegram.WebApp.showAlert("Cannot go over " + positionInput.max + "!");
    return;
  }
  fetch('/api/new_reservation', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      position: positionInput.valueAsNumber,
      line_id: lineId
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    Telegram.WebApp.showAlert(data.message);
    getReservations(lineId);
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert("Server error: " + error.message);
  });

  reserveDialog.close();
}

const removalConfirmationDialog = document.getElementById("removalConfirmationDialog");

function openRemovalConfirmationDialog() {
  fetch('/api/get_reservations', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      line_id: lineId,
      is_my_reservation: true
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    if (data.reservation === null) {
      Telegram.WebApp.showAlert("You have no active reservations!");
      return;
    }

    const positionToDelete = document.getElementById("positionToDelete");

    positionToDelete.innerText = "Position: " + data.reservation.position;

    removalConfirmationDialog.showModal();
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert("Server error: " + error.message);
  });
}

function removeMySpot() {
  fetch('/api/remove_reservation', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      line_id: lineId
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    Telegram.WebApp.showAlert(data.message);
    getReservations(lineId);
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert("Server error: " + error.message);
  });

  removalConfirmationDialog.close();
}
