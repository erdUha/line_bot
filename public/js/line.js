
const tg = window.Telegram.WebApp;
tg.ready();
tg.expand();

const params = new URLSearchParams(window.location.search);

const lineId = Number(params.get('line_id'));

const lineName = document.getElementById('lineName');
const courseName = document.getElementById('courseName');
const creator = document.getElementById('creator');

const userId = tg.initDataUnsafe?.user?.id;
const username = tg.initDataUnsafe?.user?.username;
const firstName = tg.initDataUnsafe?.user?.first_name;
const lastName = tg.initDataUnsafe?.user?.last_name;

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
    if (userId === data.line.creator_id) {
      document.getElementById("removeLineButton").style.display = "inline-block";
    }
  }).catch(error => {
    console.error("Failed to retireve data: ", error);
    Telegram.WebApp.showAlert("Failed to get line data!: " + error);
  });
}

getLine(lineId);

function removeLine() {
  fetch('/api/remove_line', {
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
    window.location.href = '/';
  }).catch(error => {
    console.error("Failed to retireve data: ", error);
    Telegram.WebApp.showAlert("Failed to remove line!: " + error);
  });
}

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

const lineRemovalConfirmationDialog = document.getElementById("lineRemovalConfirmationDialog");

function openLineRemovalConfirmationDialog() {
  lineRemovalConfirmationDialog.showModal();
}

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

const reservationRemovalConfirmationDialog = document.getElementById("reservationRemovalConfirmationDialog");

function openReservationRemovalConfirmationDialog() {
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

    reservationRemovalConfirmationDialog.showModal();
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

  reservationRemovalConfirmationDialog.close();
}
