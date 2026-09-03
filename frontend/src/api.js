const GATEWAY_BASE_URL = process.env.GATEWAY_BASE_URL;

export const postJson = async (path, payload) => {
  let response;
  try {
    response = await fetch(`${GATEWAY_BASE_URL}${path}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });
  } catch {
    throw new Error('Не удалось отправить данные на сервер. Попробуйте позже.');
  }

  if (!response.ok) {
    const message = (await response.text()).trim();
    throw new Error(message || `Ошибка ${response.status}`);
  }

  return response.json();
};

export const authGet = async (path, token) => {
  let response;
  try {
    response = await fetch(`${GATEWAY_BASE_URL}${path}`, {
      headers: { Authorization: `Bearer ${token}` },
    });
  } catch {
    throw new Error('Не удалось отправить данные на сервер. Попробуйте позже.');
  }

  if (!response.ok) {
    const message = (await response.text()).trim();
    throw new Error(message || `Ошибка ${response.status}`);
  }

  return response.json();
};
