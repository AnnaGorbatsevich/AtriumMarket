import React, { createContext, useCallback, useContext, useEffect, useState } from 'react';
import { authGet } from './api';

const TOKEN_KEY = 'atrium_token';

const AuthContext = createContext(null);

export const AuthProvider = ({ children }) => {
  const [status, setStatus] = useState('loading');
  const [user, setUser] = useState(null);

  const verify = useCallback(async (token) => {
    try {
      const profile = await authGet('/me', token);
      setUser(profile);
    } catch {
      localStorage.removeItem(TOKEN_KEY);
      setUser(null);
    } finally {
      setStatus('ready');
    }
  }, []);

  useEffect(() => {
    const token = localStorage.getItem(TOKEN_KEY);
    if (token) {
      verify(token);
    } else {
      setStatus('ready');
    }
  }, [verify]);

  const login = useCallback(
    async (token) => {
      localStorage.setItem(TOKEN_KEY, token);
      setStatus('loading');
      await verify(token);
    },
    [verify]
  );

  const logout = useCallback(() => {
    localStorage.removeItem(TOKEN_KEY);
    setUser(null);
  }, []);

  return <AuthContext.Provider value={{ status, user, login, logout }}>{children}</AuthContext.Provider>;
};

export const useAuth = () => useContext(AuthContext);
