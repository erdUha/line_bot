const tg = window.Telegram.WebApp;
tg.ready();
tg.expand();

const userId = tg.initDataUnsafe?.user?.id;
const username = tg.initDataUnsafe?.user?.username;
const firstName = tg.initDataUnsafe?.user?.first_name;
const lastName = tg.initDataUnsafe?.user?.last_name;

linesTableBody = document.getElementById("linesTable").tBodies[0];

function getLines() {
  fetch('/api/get_lines', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    linesTableBody.innerHTML = '';
    data.lines.forEach(line => {
      let row = linesTableBody.insertRow();
      row.insertCell().textContent = line.course_name;
      row.insertCell().textContent = line.line_name;
      row.insertCell().textContent = line.defence_date;
      let newButton = document.createElement('BUTTON');
      newButton.innerText = "->";
      newButton.onclick = function() {redirectLine(line.line_id, line.is_group_work)};
      row.insertCell().appendChild(newButton);
    });
  }).catch(error => {
    console.error("Failed to retireve data: ", error);
    Telegram.WebApp.showAlert("Failed to get line data! " + error);
  });
}

if (userId)
{
  fetch('/api/app_opened', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status === "registration") {
      openRegisterDialog(data.admin_username);
      return;
    }
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    getLines();
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert(error);
  });
} else
{
  console.log("Not running inside Telegram.");
}

const registerInfo = document.getElementById("registerInfo");
const registerDialog = document.getElementById("registerDialog");

function openRegisterDialog(adminUsername) {
  registerInfo.innerText = "Note: ask @" + adminUsername + " for group password";
  registerDialog.showModal();
}

function register() {
  const group_password = document.getElementById("groupPassword");
  const firstName = document.getElementById("firstName");
  const lastName = document.getElementById("lastName");

  fetch('/api/register', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      group_password: group_password.value,
      first_name: firstName.value.trim(),
      last_name: lastName.value.trim()
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status === "wrong") {
      Telegram.WebApp.showAlert(data.message);
    }
    if (data.status === "success") {
      registerDialog.close();
      Telegram.WebApp.showAlert("Successfully registered!");
    }
    throw new Error(data.message || "Unknown server error");
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert("Failed to register!: " + error);
  });
}

const newLineDialog = document.getElementById("newLineDialog");

function openLineDialog() {
  newLineDialog.showModal();
}

function redirectLine(lineId, isGroupWork) {
  window.location.href = "/line?line_id=" + lineId + "&is_group_work=" + isGroupWork;
}

function createLine() {
  const courseName = document.getElementById("courseName").value;
  const lineName = document.getElementById("lineName").value;
  const isGroupWork = document.getElementById("isGroupWork").checked;
  const defenceDate = document.getElementById("defenceDate").value;

  fetch('/api/new_line', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      course_name: courseName,
      line_name: lineName,
      is_group_work: isGroupWork,
      defence_date: defenceDate
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    Telegram.WebApp.showAlert(data.message);
    getLines();
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert("Server error: " + error.message);
  });

  newLineDialog.close();
}
