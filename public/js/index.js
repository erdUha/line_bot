const tg = window.Telegram.WebApp;
tg.ready();
tg.expand();

const userId = tg.initDataUnsafe?.user?.id;
const username = tg.initDataUnsafe?.user?.username;
const firstName = tg.initDataUnsafe?.user?.first_name;
const lastName = tg.initDataUnsafe?.user?.last_name;

is_admin = false;

newUserButton = document.getElementById("newUserButton");
responseH3 = document.getElementById("response");

//document.getElementById("registerDialog").showModal();

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
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    is_admin = data.is_admin;
    if (is_admin) {
      newUserButton.style.display = "inline-block";
    }
    responseH3.innerText = "is_admin: " + data.is_admin;
  }).catch(error => {
    console.error("Failed to send: ", error);
  });
} else
{
  console.log("Not running inside Telegram.");
}

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
    responseH3.innerText = data.lines.length;
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

getLines();

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

const newUserDialog = document.getElementById("newUserDialog");

function openNewUserDialog() {
  newUserDialog.showModal();
}

function addUser() {
  if (!is_admin) {
    Telegram.WebApp.showAlert("Not an admin!");
    return;
  }

  const newUserTgId = Number(document.getElementById("newUserTgId").value);
  const newUserName = document.getElementById("newUserName").value;
  const newUserFirstName = document.getElementById("newUserFirstName").value;
  const newUserLastName = document.getElementById("newUserLastName").value;

  fetch('/api/admin/new_user', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      initData: tg.initData,
      new_user: {
        telegram_id: newUserTgId,
        username: newUserName,
        first_name: newUserFirstName,
        last_name: newUserLastName
      }
    })
  }).then(res => {
    return res.json();
  }).then(data => {
    if (data.status !== "success") {
      throw new Error(data.message || "Unknown server error");
    }
    Telegram.WebApp.showAlert(data.message);
  }).catch(error => {
    console.error("Failed to send: ", error);
    Telegram.WebApp.showAlert("Server error: " + error.message);
  });
}
