import React, { useState } from 'react';
import LoginForm from './LoginForm';

const LoginPage = () => {
  const [user, setUser] = useState(null);

  if (user) {
    return (
      <div className="container">
        <div className="card">
          <div className="header">
            <h1>Вы вошли</h1>
          </div>
          <div className="card-body" style={{ padding: '2rem', textAlign: 'center' }}>
            <p className="success-subtext">{user.email}</p>
            <button className="btn" onClick={() => setUser(null)}>Выйти</button>
          </div>
        </div>
      </div>
    );
  }

  return <LoginForm onSubmitted={setUser} />;
};

export default LoginPage;
