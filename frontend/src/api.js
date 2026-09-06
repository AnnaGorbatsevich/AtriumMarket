const GATEWAY_BASE_URL = process.env.GATEWAY_BASE_URL;

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
