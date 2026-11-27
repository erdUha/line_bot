
const tg = window.Telegram.WebApp;
tg.ready();
tg.expand();

const params = new URLSearchParams(window.location.search);

const lineId = Number(params.get('line_id'));

let is_group_work = false;

const lineName = document.getElementById('lineName');
const courseName = document.getElementById('courseName');
const creator = document.getElementById('creator');

const userId = tg.initDataUnsafe?.user?.id;
const username = tg.initDataUnsafe?.user?.username;
const firstName = tg.initDataUnsafe?.user?.first_name;
const lastName = tg.initDataUnsafe?.user?.last_name;

reservationsTableBody = document.getElementById("reservationsTable").tBodies[0];
removeMySpotButton = document.getElementById("removeMySpotButton");

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

    if (data.my_reservation === null) {
      removeMySpotButton.style.display = "none";
    }
    else {
      removeMySpotButton.style.display = "inline-block";
    }

    reservationsTableBody.innerHTML = '';
    data.reservations.forEach(reserv => {
      let row = reservationsTableBody.insertRow();
      row.insertCell().textContent = reserv.position;
      const cell = row.insertCell();
      cell.style.whiteSpace = "pre-line";
      let groupStr = reserv.reserved_by.map(user => {
        let initial = user.last_name ? ` ${user.last_name[0]}.` : "";
        return user.first_name + initial;
      }).join(",\n");
      cell.textContent = groupStr;
      if (is_group_work === true) {
        let newButton = document.createElement('BUTTON');
        newButton.innerText = "+";
        newButton.onclick = function() {joinGroup(reserv.position);};
        row.insertCell().appendChild(newButton);
      }
    });
  }).catch(error => {
    console.error("Failed to retireve data: ", error);
    Telegram.WebApp.showAlert("Failed to get line data!: " + error);
  });
}

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
    is_group_work = data.line.is_group_work;
    if (is_group_work === true) {
      document.getElementById("studentColumn").innerText = "Student(s)";
      document.getElementById("joinGroupColumn").style.display = "block";
    }
    getReservations(lineId);
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

function joinGroup(position) {
  fetch('/api/new_reservation', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      position: position,
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
}

const lineRemovalConfirmationDialog = document.getElementById("lineRemovalConfirmationDialog");

function openLineRemovalConfirmationDialog() {
  lineRemovalConfirmationDialog.showModal();
}

const reserveDialog = document.getElementById("reserveDialog");

function openReserveDialog() {
  reserveDialog.showModal();
}

function createReservation() {
  if (Number.isNaN(positionInput.valueAsNumber)) {
    Telegram.WebApp.showAlert("Enter the position to reserve!");
    return;
  }
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
