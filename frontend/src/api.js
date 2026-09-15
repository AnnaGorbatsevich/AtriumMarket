// Gateway always runs on port 8080 of the same host that serves the frontend.
const GATEWAY_BASE_URL = `${window.location.protocol}//${window.location.hostname}:8080`;

const request = async (path, options) => {
  let response;
  try {
    response = await fetch(`${GATEWAY_BASE_URL}${path}`, options);
  } catch {
    throw new Error('Не удалось отправить данные на сервер. Попробуйте позже.');
  }

  if (!response.ok) {
    const message = (await response.text()).trim();
    throw new Error(message || `Ошибка ${response.status}`);
  }

  return response.json();
};

export const getJson = (path) => request(path, {});

export const postJson = (path, payload) =>
  request(path, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  });

export const authGet = (path, token) =>
  request(path, {
    headers: { Authorization: `Bearer ${token}` },
  });

export const authPost = (path, token, payload) =>
  request(path, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', Authorization: `Bearer ${token}` },
    body: JSON.stringify(payload),
  });
